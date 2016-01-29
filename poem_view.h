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

#ifndef PHP_POEM_VIEW_H
#define PHP_POEM_VIEW_H


#define REGEX_LEN 1
#define STORE_EG_ENVIRON()\
{\
  zval ** __old_return_value_pp = EG(return_value_ptr_ptr);\
  zend_op ** __old_opline_ptr   = EG(opline_ptr);\
  zend_op_array * __old_op_array= EG(active_op_array);

#define RESTORE_EG_ENVIRON()\
  EG(return_value_ptr_ptr) = __old_return_value_pp;\
  EG(opline_ptr) = __old_opline_ptr;\
  EG(active_op_array) = __old_op_array;\
}
#endif

POEM_GO_MF(view);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
