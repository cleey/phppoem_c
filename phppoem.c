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
#include "php_phppoem.h"

#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"  
#include "ext/standard/php_filestat.h"

#include "main/php_streams.h"

#include "poem_route.h"
#include "poem_view.h"

zend_class_entry *phppoem_ce;
static int le_phppoem;

zend_class_entry *get_class_entry(char *class_name, int class_name_len TSRMLS_DC){
	zend_class_entry **ce;
	if (zend_lookup_class(class_name, class_name_len, &ce TSRMLS_CC) == SUCCESS) {
		return *ce;
	} else {
		return NULL;
	}
}

zval * poem_instance(zval *this_ptr TSRMLS_DC){
	zval *instance;
	instance = zend_read_static_property(phppoem_ce, ZEND_STRL("instance"), 0 TSRMLS_CC);
	if(IS_OBJECT == Z_TYPE_P(instance)
			&& instanceof_function(Z_OBJCE_P(instance), phppoem_ce TSRMLS_CC) )
		return instance;

	if( this_ptr ){
		instance = this_ptr;
		return instance;
	}else{
		instance = NULL;
		MAKE_STD_ZVAL(instance);
		object_init_ex(instance, phppoem_ce);
	}

	zend_update_static_property(phppoem_ce, ZEND_STRL("instance"), instance TSRMLS_CC);

	return instance;
}

// 引用php文件
int poem_include(char *path){
	zend_file_handle file_handle;
	zend_op *op_array;
	HashTable *calling_symbol_table;

	if( zend_stream_open(path, &file_handle TSRMLS_CC) != SUCCESS ){
		zend_error(E_WARNING, "file %s : can not open", path);
		return FAILURE;
	}

	op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
	if( op_array && file_handle.handle.stream.handle ){
		int dummy = 1;
		zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1, (void *)&dummy, sizeof(int), NULL);
	}
	zend_destroy_file_handle(&file_handle TSRMLS_CC);

	if( op_array ){
		zval *result = NULL;
		STORE_EG_ENVIRON();

		EG(return_value_ptr_ptr) = &result;
		EG(active_op_array) = op_array;
		if( !EG(active_symbol_table) ){
			zend_rebuild_symbol_table(TSRMLS_CC);
		}

		zend_execute(op_array TSRMLS_CC);
		destroy_op_array(op_array TSRMLS_CC);
		efree(op_array);

		if( !EG(exception)) {
			if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
				zval_ptr_dtor(EG(return_value_ptr_ptr));
			}
		}
		RESTORE_EG_ENVIRON();
		return SUCCESS;
	}

	return FAILURE;
}


int poem_autoload(char *filename,uint name_len TSRMLS_DC){
	char *act = "app/home/controller/index.php";
	return poem_include(act);
	// return yaf_loader_import(act, strlen(act), 0 TSRMLS_CC);
}

void poem_run(char *ctrl_path,char *ctrl_name,char *ctrl_func){
	zend_class_entry *ce;
	zend_function *func;
	zval *iaction, *ret;

	// class_lowercase = zend_str_tolower_dup(class_name, class_len);
	poem_include(ctrl_path);
	ce = get_class_entry(ctrl_name, strlen(ctrl_name) );

	if( ce == NULL ){
		php_printf("%s cannot find\n",ctrl_name);
		return;
	}

	if( zend_hash_find( &((ce)->function_table), ctrl_func, strlen(ctrl_func)+1, (void **)&func) != SUCCESS ){
		php_printf("index function is not exit\n");
		return;
	}

	MAKE_STD_ZVAL(iaction);
	object_init_ex(iaction, ce);

	zend_call_method(&iaction, ce, NULL, ctrl_func, strlen(ctrl_func), NULL, 0, NULL, NULL TSRMLS_CC);

	zval_ptr_dtor(&ce);
	zval_ptr_dtor(&iaction);
	zval_ptr_dtor(&ret);

	return;
}

PHP_METHOD(phppoem,__construct){
	zval *route;
	// php_printf("phppoem __construct ok \n");
	route = poem_route_instance(NULL TSRMLS_CC);
}
PHP_METHOD(phppoem,run){
	char ctrl_path[255], ctrl_name[255], ctrl_func[127];

	poem_route_run(); // 路由
	poem_route_ctrl_path(ctrl_path);
	poem_route_ctrl_name(ctrl_name);
	poem_route_ctrl_func(ctrl_func);
	poem_run(ctrl_path, ctrl_name, ctrl_func);

	RETURN_TRUE;
}

// 定义所有PHP使用的的函数
const zend_function_entry phppoem_functions[] = { 
	PHP_ME(phppoem, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(phppoem, run, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_FE_END
};

// 注册所有的函数
zend_module_entry phppoem_module_entry = {
	STANDARD_MODULE_HEADER,
	"phppoem",
	phppoem_functions,
	PHP_MINIT(phppoem),
	PHP_MSHUTDOWN(phppoem),
	PHP_RINIT(phppoem),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(phppoem),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(phppoem),
	PHP_PHPPOEM_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHPPOEM
ZEND_GET_MODULE(phppoem)
#endif


// php启动时，初始化类
PHP_MINIT_FUNCTION(phppoem){
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "phppoem", phppoem_functions);
	phppoem_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	POEM_GO(route);
	POEM_GO(view);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(phppoem){return SUCCESS;}
PHP_RINIT_FUNCTION(phppoem){return SUCCESS;}
PHP_RSHUTDOWN_FUNCTION(phppoem){return SUCCESS;}

PHP_MINFO_FUNCTION(phppoem){
	php_info_print_table_start();
	php_info_print_table_header(2, "phppoem support", "enabled");
	php_info_print_table_end();
}



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
