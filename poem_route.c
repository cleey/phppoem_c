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
#include "poem_route.h"

zend_class_entry *poem_route_ce;
static int le_poem_route;

zval * poem_route_instance(zval *this_ptr TSRMLS_DC){
	zval *instance;
	instance = zend_read_static_property(poem_route_ce, ZEND_STRL("instance"), 1 TSRMLS_CC);
	if(IS_OBJECT == Z_TYPE_P(instance) && instanceof_function(Z_OBJCE_P(instance), poem_route_ce TSRMLS_CC) ){
		// zval *info;
		// info = zend_read_property(poem_route_ce,instance,ZEND_STRL("module"),1 TSRMLS_CC);
		// php_printf("module_test => %s\n",Z_STRVAL_P(info));

		return instance;
	}

	if( this_ptr ){
		instance = this_ptr;
		return instance;
	}else{
		instance = NULL;
		MAKE_STD_ZVAL(instance);
		object_init_ex(instance, poem_route_ce);
	}

	zend_update_static_property(poem_route_ce, ZEND_STRL("instance"), instance TSRMLS_CC);

	zval *library;
	MAKE_STD_ZVAL(library);
	ZVAL_STRING(library, "test", 1);
	zend_update_property(poem_route_ce, instance, ZEND_STRL("module"), library TSRMLS_CC);

	return instance;
}

zval * poem_global_vars(uint type, char * name, uint len TSRMLS_DC) {
	zval 		**carrier = NULL, **ret;
	zend_bool 	jit_initialization = PG(auto_globals_jit);

	switch (type) {
		case POEM_G_VARS_POST:
			(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_POST"), (void **)&carrier);
			break;
		case POEM_G_VARS_GET:
			(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&carrier);
			break;
		case POEM_G_VARS_COOKIE:
			(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_COOKIE"), (void **)&carrier);
			break;
		case POEM_G_VARS_SERVER:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
			}
			(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_SERVER"), (void **)&carrier);
			break;
		case POEM_G_VARS_ENV:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_ENV") TSRMLS_CC);
			}
			carrier = &PG(http_globals)[type];
			break;
		case POEM_G_VARS_FILES:
			carrier = &PG(http_globals)[type];
			break;
		case POEM_G_VARS_REQUEST:
			if (jit_initialization) {
				zend_is_auto_global(ZEND_STRL("_REQUEST") TSRMLS_CC);
			}
			(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_REQUEST"), (void **)&carrier);
			break;
		default:
			break;
	}

	if (!carrier || !(*carrier)) {
		zval *empty;
		MAKE_STD_ZVAL(empty);
		ZVAL_NULL(empty);
		return empty;
	}

	if (!len) {
		Z_ADDREF_P(*carrier);
		return *carrier;
	}

	if (zend_hash_find(Z_ARRVAL_PP(carrier), name, len + 1, (void **)&ret) == FAILURE) {
		zval *empty;
		MAKE_STD_ZVAL(empty);
		ZVAL_NULL(empty);
		return empty;
	}

	Z_ADDREF_P(*ret);
	return *ret;
}

void poem_route_run(){
	zval *module, *ctrl, *func, *route;
	zval *split, *path_arr, *new_path_arr, *request_uri;
	long path_num;

	request_uri = poem_global_vars(POEM_G_VARS_SERVER, ZEND_STRL("PATH_INFO"));

	if(Z_STRVAL_P(request_uri) != NULL ){
		MAKE_STD_ZVAL(split);
		ZVAL_STRINGL(split, "/", 1, 0);
		MAKE_STD_ZVAL(path_arr);
		array_init(path_arr);
		php_explode(split, request_uri, path_arr, 20);
	}

	if(Z_STRVAL_P(request_uri) != NULL && (path_num = (zend_hash_num_elements(Z_ARRVAL_P(path_arr))-1)) ){
		zval **module_p, **ctrl_p, **func_p;

		switch( path_num ){
			case 1:
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 1, (void**)&module_p);
				MAKE_STD_ZVAL(ctrl);
				MAKE_STD_ZVAL(func);
				ZVAL_STRINGL(ctrl, "index", 5, 1);
				ZVAL_STRINGL(func, "index", 5, 1);
				module = *module_p;
				break;
			case 2:
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 1, (void**)&module_p);
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 2, (void**)&ctrl_p);
				MAKE_STD_ZVAL(func);
				ZVAL_STRINGL(func, "index", 5, 1);
				module = *module_p;
				ctrl   = *ctrl_p;
				break;
			default:
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 1, (void**)&module_p);
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 2, (void**)&ctrl_p);
				zend_hash_index_find(Z_ARRVAL_P(path_arr), 3, (void**)&func_p);
				module = *module_p;
				ctrl   = *ctrl_p;
				func   = *func_p;
				break;
		}
	}else{
		MAKE_STD_ZVAL(module);
		MAKE_STD_ZVAL(ctrl);
		MAKE_STD_ZVAL(func);
		ZVAL_STRINGL(module, "home", 4, 1);
		ZVAL_STRINGL(ctrl, "index", 5, 1);
		ZVAL_STRINGL(func, "index", 5, 1);
	}
	// php_printf("route(%d) => %s|%s|%s\n",path_num,Z_STRVAL_P(module),Z_STRVAL_P(ctrl),Z_STRVAL_P(func));

	route = poem_route_instance(NULL TSRMLS_CC);
	zend_update_property(poem_route_ce, route, ZEND_STRL("module"), module TSRMLS_CC);
	zend_update_property(poem_route_ce, route, ZEND_STRL("ctrl"), ctrl TSRMLS_CC);
	zend_update_property(poem_route_ce, route, ZEND_STRL("func"), func TSRMLS_CC);
}

// 控制器名
void poem_route_ctrl_name(char *path){
	zval *route, *module ,*ctrl;
	route = poem_route_instance(NULL TSRMLS_CC);
	module = zend_read_property(poem_route_ce,route,ZEND_STRL("module"),1 TSRMLS_CC);
	ctrl = zend_read_property(poem_route_ce,route,ZEND_STRL("ctrl"),1 TSRMLS_CC);

	php_sprintf(path, "\\%s\\controller\\%s", Z_STRVAL_P(module), Z_STRVAL_P(ctrl) );
}
// 控制器路径
void poem_route_ctrl_path(char *path){
	// char path[255], *p;
	zval *route, *module ,*ctrl;
	route = poem_route_instance(NULL TSRMLS_CC);
	module = zend_read_property(poem_route_ce,route,ZEND_STRL("module"),1 TSRMLS_CC);
	ctrl = zend_read_property(poem_route_ce,route,ZEND_STRL("ctrl"),1 TSRMLS_CC);

	php_sprintf(path, "app/%s/controller/%s.php", Z_STRVAL_P(module), Z_STRVAL_P(ctrl) );
}
void poem_route_ctrl_func(char *path){
	zval *route, *func;
	route = poem_route_instance(NULL TSRMLS_CC);
	func = zend_read_property(poem_route_ce,route,ZEND_STRL("func"),1 TSRMLS_CC);

	php_sprintf(path, "%s", Z_STRVAL_P(func) );
}

PHP_METHOD(poem_route,getInstance){
	zval *route = poem_route_instance(NULL TSRMLS_CC);
	RETURN_ZVAL(route,0,0);
}

// 定义所有PHP使用的的函数
const zend_function_entry poem_route_functions[] = { 
	PHP_ME(poem_route, getInstance, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

// php启动时，初始化类
POEM_GO_MF(route){
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "poem_route", poem_route_functions);
	poem_route_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_declare_property_null(poem_route_ce, ZEND_STRL("instance"), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC TSRMLS_CC);

	zend_declare_property_string(poem_route_ce, ZEND_STRL("module"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(poem_route_ce, ZEND_STRL("ctrl"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(poem_route_ce, ZEND_STRL("func"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(poem_route_ce, ZEND_STRL("ctrl_name"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(poem_route_ce, ZEND_STRL("ctrl_path"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
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
