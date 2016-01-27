dnl $Id$
dnl config.m4 for extension phppoem

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(phppoem, for phppoem support,
dnl Make sure that the comment is aligned:
dnl [  --with-phppoem             Include phppoem support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(phppoem, whether to enable phppoem support,
Make sure that the comment is aligned:
[  --enable-phppoem           Enable phppoem support])

if test "$PHP_PHPPOEM" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-phppoem -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/phppoem.h"  # you most likely want to change this
  dnl if test -r $PHP_PHPPOEM/$SEARCH_FOR; then # path given as parameter
  dnl   PHPPOEM_DIR=$PHP_PHPPOEM
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for phppoem files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHPPOEM_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHPPOEM_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the phppoem distribution])
  dnl fi

  dnl # --with-phppoem -> add include path
  dnl PHP_ADD_INCLUDE($PHPPOEM_DIR/include)

  dnl # --with-phppoem -> check for lib and symbol presence
  dnl LIBNAME=phppoem # you may want to change this
  dnl LIBSYMBOL=phppoem # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHPPOEM_DIR/$PHP_LIBDIR, PHPPOEM_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHPPOEMLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong phppoem lib version or lib not found])
  dnl ],[
  dnl   -L$PHPPOEM_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHPPOEM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(phppoem, phppoem.c, $ext_shared)
fi
