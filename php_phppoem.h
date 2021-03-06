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

#ifndef PHP_PHPPOEM_H
#define PHP_PHPPOEM_H

extern zend_module_entry phppoem_module_entry;
#define phpext_phppoem_ptr &phppoem_module_entry

#define FETCH_THIS Z_OBJCE_P(getThis()), getThis()
#define POEM_GO(module) PHP_MINIT(poem_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define POEM_GO_MF(module) PHP_MINIT_FUNCTION(poem_##module)

#define PHP_PHPPOEM_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_PHPPOEM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PHPPOEM_API __attribute__ ((visibility("default")))
#else
#	define PHP_PHPPOEM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define PHPPOEM_G(v) TSRMG(phppoem_globals_id, zend_phppoem_globals *, v)
#else
#define PHPPOEM_G(v) (phppoem_globals.v)
#endif

#endif	/* PHP_PHPPOEM_H */
PHP_MINIT_FUNCTION(phppoem);
PHP_MSHUTDOWN_FUNCTION(phppoem);
PHP_RINIT_FUNCTION(phppoem);
PHP_RSHUTDOWN_FUNCTION(phppoem);
PHP_MINFO_FUNCTION(phppoem);




/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
