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
  | Author: cleey                                                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_phppoem.h"

#include "poem_view.h"
#include "poem_view.c"

zend_class_entry *phppoem_ce;
static int le_phppoem;

// 定义所有PHP使用的的函数
const zend_function_entry phppoem_functions[] = { PHP_FE_END };

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
