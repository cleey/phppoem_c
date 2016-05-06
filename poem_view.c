/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: cleey  <cleeytest@163.com>                                   |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"

#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"  
#include "ext/standard/php_filestat.h"

#include "main/php_streams.h"

#include "ext/pcre/php_pcre.h"
#include "php_phppoem.h"
#include "poem_view.h"

zend_class_entry *poem_view_ce;
static int le_poem_view;

// 获取参数的函数，都需要在这里声明获取多少个参数
ZEND_BEGIN_ARG_INFO(arg_poem_view_setdir,0)
	ZEND_ARG_INFO(0,dir)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arg_poem_view_assign,0)
	ZEND_ARG_INFO(0,key)
	ZEND_ARG_INFO(0,value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arg_poem_view_display,0)
	ZEND_ARG_INFO(0,tpl)
ZEND_END_ARG_INFO()


// ---------- c函数Begin
static int create_folder(char *fullpath){
	php_stream_statbuf ssb;
	if( FAILURE == php_stream_stat_path(fullpath, &ssb) ){
		if( !php_stream_mkdir(fullpath, 0755, PHP_STREAM_MKDIR_RECURSIVE, NULL) ){
			zend_error(E_ERROR, "could not create directory \"%s\"", fullpath);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static int poew_view_compile_view(php_stream *stream, char *compile_path){
	zval *replace_val;
	char *result;
	int  result_len, i;

	char regex[REGEX_LEN][100] = {
		// "/\\{\\:(.*)\$\\}/s", // {:func($vo['info'])}
		"/\\<if\\s+(.+?)\\>/is",
		"/\\<elseif\\s+(.+?)\\>/is",
		"/\\<else \\/\\>/is",
		"/\\<\\/if\\>/is",
		"/\\{(\\$[\\w\\[\\]'\"\$]+)\\}/s", // {$helo}
		"/h/s"
	};
	char replace[REGEX_LEN][100] = {
		// "<?php echo '\\1'; ?>",
		"<?php if(\\1) { ?>",
		"<?php } elseif(\\1) { ?>",
		"<?php } else { ?>",
		"<?php } ?>",
		"<?php echo \\1; ?>",
		"hi"
	};

	// 获取输入文件内容
	char buf[1024];
	smart_str content = {0};
	while(!php_stream_eof(stream)){
		if( !php_stream_gets(stream,buf,1024) ) break;
		smart_str_appends(&content, buf);
	}
	php_stream_close(stream);
	smart_str_0(&content); //  如果字符串存在，给字符串的最后添加’\0’;

	//  这个宏会用内核的方式来申请一块内存并将其地址付给 replace_val， 并初始化它的refcount和is_ref两个属性，更棒的是，它不但会自动的处理内存不足问题， 还会在内存中选个最优的位置来申请。
	MAKE_STD_ZVAL(replace_val);
	// result = content.c;

	// 开始正则替换
	for (i = 0; i < REGEX_LEN; i++){
		ZVAL_STRINGL(replace_val, replace[i], strlen(replace[i]), 1);
		//  php_pcre_replace会返回一个malloc内存的result
		if( (result = php_pcre_replace(regex[i], strlen(regex[i]), 
										content.c, content.len, 
										replace_val, 0,
										&result_len, -1, NULL TSRMLS_CC)) != NULL ){
			content.c = result;
			content.len = result_len;
		}
	}

	if( result == NULL ){
		zend_error(E_WARNING, "%s compiler is failed", compile_path);
		result = content.c;
	}

	stream = php_stream_open_wrapper(compile_path, "wb", REPORT_ERRORS|ENFORCE_SAFE_MODE, NULL);
	if( stream == NULL ) {
		zend_error(E_WARNING, "%s does not write able", compile_path);
		return FAILURE;
	}
	php_stream_write_string(stream, result);
	php_stream_close(stream);

	efree(result);

	return SUCCESS;
}

static void poew_view_exec_view(zend_file_handle *file_handle, zval *assign) {
	zend_op *op_array;
	HashTable *calling_symbol_table;
	// 编译文件获取opcode数组
	op_array = zend_compile_file(file_handle, ZEND_REQUIRE TSRMLS_CC);
	if(op_array && file_handle->handle.stream.handle){
		int dummy = 1;
		zend_hash_add(&EG(included_files), file_handle->opened_path, strlen(file_handle->opened_path)+1, (void *)&dummy, sizeof(int), NULL);
	}
	zend_destroy_file_handle(file_handle TSRMLS_CC);

	// 将符号表内容临时保存
	if( EG(active_symbol_table) ){ calling_symbol_table = EG(active_symbol_table); }
	else{ calling_symbol_table = NULL; }

	ALLOC_HASHTABLE(EG(active_symbol_table));
	zend_hash_init(EG(active_symbol_table), 0, NULL, ZVAL_PTR_DTOR, 0);

	// 将assign数组写入符号表
	if( Z_TYPE_P(assign) == IS_ARRAY ){ 
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(assign));
		while( zend_hash_has_more_elements(Z_ARRVAL_P(assign)) == SUCCESS ){
			char *key;
			int key_len;
			ulong idx;
			zval **data;
			if( zend_hash_get_current_key_ex(Z_ARRVAL_P(assign), &key, &key_len, &idx,0 ,NULL) != HASH_KEY_IS_STRING ) continue;
			if( zend_hash_get_current_data(Z_ARRVAL_P(assign) ,(void **)&data) == FAILURE ) continue;
			// 写入符号表
			ZEND_SET_SYMBOL_WITH_LENGTH(EG(active_symbol_table), key, key_len, *data, Z_REFCOUNT_P(*data)+1, PZVAL_IS_REF(*data));
			zend_hash_move_forward(Z_ARRVAL_P(assign));
		}
	}

	// 执行view文件
	if( op_array ){
		zval *result = NULL; 
		STORE_EG_ENVIRON();

		EG(return_value_ptr_ptr) = &result;
		EG(active_op_array) = op_array;

		if( !EG(active_op_array) ){ zend_rebuild_symbol_table(TSRMLS_CC); }

		zend_execute(op_array TSRMLS_CC);
		destroy_op_array(op_array TSRMLS_CC);
		efree(op_array);

		if( !EG(exception) ){
			if( EG(return_value_ptr_ptr) )
				zval_ptr_dtor(EG(return_value_ptr_ptr));
		}

		RESTORE_EG_ENVIRON();
	}

	if( calling_symbol_table ){
		FREE_HASHTABLE(EG(active_symbol_table));
		EG(active_symbol_table) = calling_symbol_table;
	}
}

static void poew_view_display(INTERNAL_FUNCTION_PARAMETERS, int is_obstart){
	zval *tpl, *assign, *compile_dir, *template_dir, tpl_exists, cmp_exists;
	char *cpath;
	php_stream *stream;
	zend_file_handle file_handle;

	smart_str compile_path={0};
	smart_str template_path={0};

	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &tpl) == FAILURE ) RETURN_FALSE;

	assign = zend_read_property(FETCH_THIS, ZEND_STRL("assign"), 0 TSRMLS_CC);
	template_dir = zend_read_property(FETCH_THIS, ZEND_STRL("template_dir"), 0 TSRMLS_CC);
	compile_dir = zend_read_property(FETCH_THIS, ZEND_STRL("compile_dir"), 0 TSRMLS_CC);

	// 编译路径
	smart_str_appendl(&compile_path, Z_STRVAL_P(compile_dir), Z_STRLEN_P(compile_dir));
	smart_str_appendl(&compile_path, Z_STRVAL_P(tpl), Z_STRLEN_P(tpl));
	smart_str_appendl(&compile_path, ".php", 4);
	smart_str_0(&compile_path);

	cpath = estrndup(compile_path.c,compile_path.len);
	php_dirname(cpath, compile_path.len);
	if( create_folder(cpath) == FAILURE ) RETURN_FALSE;

	// 模板路径
	smart_str_appendl(&template_path, Z_STRVAL_P(template_dir), Z_STRLEN_P(template_dir));
	smart_str_appendl(&template_path, Z_STRVAL_P(tpl), Z_STRLEN_P(tpl));
	smart_str_appendl(&template_path, ".html", 5);
	smart_str_0(&template_path);

	// 验证模板文件存在
	php_stat(template_path.c, template_path.len, FS_EXISTS, &tpl_exists TSRMLS_CC);
	if( Z_LVAL(tpl_exists) == 0 ){
		zend_error(E_WARNING, "file: %s not exists", template_path);
		RETURN_FALSE;
	}

	// 打开模板文件
	stream = php_stream_open_wrapper(template_path.c, "rb", REPORT_ERRORS|ENFORCE_SAFE_MODE, NULL);
	if( stream == NULL ){
		zend_error(E_WARNING, "%s does not read able", template_path.c);
		RETURN_FALSE;
	}
	poew_view_compile_view(stream, compile_path.c);

	if( zend_stream_open(compile_path.c, &file_handle TSRMLS_CC) == SUCCESS ){
		if( is_obstart ){
			if( php_output_start_default(TSRMLS_CC)==SUCCESS ){
				poew_view_exec_view(&file_handle, assign);
				php_output_get_contents(return_value TSRMLS_CC);
				php_output_end(TSRMLS_CC);
				RETURN_ZVAL(return_value, 0, NULL);
			}else RETURN_FALSE;
		}else{
			poew_view_exec_view(&file_handle, assign);
		}
	}else{
		zend_error(E_WARNING, "%s does not open able", compile_path.c);
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}

// ---------- c函数End



// ---------- 实现函数Begin 
PHP_METHOD(poem_view,setTemplateDir){
	zval *dir;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z", &dir) == FAILURE ) RETURN_FALSE;
	if( Z_TYPE_P(dir) == IS_STRING ){
		// zend_update_property(zend_class_entry *scope, zval *object, const char *name, int name_length, zval *value TSRMLS_DC);
		zend_update_property(FETCH_THIS, ZEND_STRL("template_dir"), dir TSRMLS_CC);
		RETURN_TRUE;
	}else{
		RETURN_FALSE;
	}
}
PHP_METHOD(poem_view,getTemplateDir){
	zval *dir;
	// ZEND_API zval *zend_read_property(zend_class_entry *scope, zval *object, const char *name, int name_length, zend_bool silent TSRMLS_DC);
	dir = zend_read_property(FETCH_THIS, ZEND_STRL("template_dir"), 0 TSRMLS_CC);
	RETURN_ZVAL(dir, 0, NULL);
}
PHP_METHOD(poem_view,setCompileDir){
	zval *dir;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z", &dir) == FAILURE ) RETURN_FALSE;
	if( Z_TYPE_P(dir) == IS_STRING ){
		zend_update_property(FETCH_THIS, ZEND_STRL("compile_dir"), dir TSRMLS_CC);
		RETURN_TRUE;
	}else{
		RETURN_FALSE;
	}
}
PHP_METHOD(poem_view,getCompileDir){
	zval *dir;
	dir = zend_read_property(FETCH_THIS, ZEND_STRL("compile_dir"), 0 TSRMLS_CC);
	RETURN_ZVAL(dir, 0, NULL);
}

PHP_METHOD(poem_view,assign){
	zval *key, *value, *assign;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &key, &value) == FAILURE ) RETURN_FALSE;

	assign = zend_read_property(FETCH_THIS, ZEND_STRL("assign"), 0 TSRMLS_CC);
	if( Z_TYPE_P(assign) == IS_NULL ){
		MAKE_STD_ZVAL(assign);
		array_init(assign);
	}

	convert_to_string(key);
	zval_addref_p(value);
	add_assoc_zval(assign, Z_STRVAL_P(key), value);

	zend_update_property(FETCH_THIS, ZEND_STRL("assign"), assign TSRMLS_CC);
	RETURN_TRUE;
}

PHP_METHOD(poem_view,display){
	poew_view_display(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
// ---------- 实现函数End


// 定义所有PHP使用的的函数
const zend_function_entry poem_view_functions[] = {
	PHP_ME(poem_view, setTemplateDir, arg_poem_view_setdir, ZEND_ACC_PUBLIC)
	PHP_ME(poem_view, getTemplateDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(poem_view, setCompileDir, arg_poem_view_setdir, ZEND_ACC_PUBLIC)
	PHP_ME(poem_view, getCompileDir, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(poem_view, assign, arg_poem_view_assign, ZEND_ACC_PUBLIC)
	PHP_ME(poem_view, display, arg_poem_view_display, ZEND_ACC_PUBLIC)
	PHP_FE_END	/* Must be the last line in poem_view_functions[] */
};

// php启动时，初始化类
POEM_GO_MF(view){
	zend_class_entry poem;
	INIT_CLASS_ENTRY(poem, "poem_view", poem_view_functions);
	poem_view_ce = zend_register_internal_class_ex(&poem, NULL, NULL TSRMLS_CC);
	// #define ZEND_STRL(str)  (str), (sizeof(str)-1)
	zend_declare_property_string(poem_view_ce, ZEND_STRL("template_dir"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(poem_view_ce, ZEND_STRL("compile_dir"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(poem_view_ce, ZEND_STRL("assign"), ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
