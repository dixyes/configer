dnl $Id$
dnl config.m4 for extension configer

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(configer, for configer support,
dnl Make sure that the comment is aligned:
dnl [  --with-configer             Include configer support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(configer, whether to enable configer support,
[  --enable-configer           Enable configer support])

if test "$PHP_CONFIGER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-configer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/configer.h"  # you most likely want to change this
  dnl if test -r $PHP_CONFIGER/$SEARCH_FOR; then # path given as parameter
  dnl   CONFIGER_DIR=$PHP_CONFIGER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for configer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CONFIGER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CONFIGER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the configer distribution])
  dnl fi

  dnl # --with-configer -> add include path
  dnl PHP_ADD_INCLUDE($CONFIGER_DIR/include)

  dnl # --with-configer -> check for lib and symbol presence
  dnl LIBNAME=configer # you may want to change this
  dnl LIBSYMBOL=configer # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CONFIGER_DIR/$PHP_LIBDIR, CONFIGER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CONFIGERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong configer lib version or lib not found])
  dnl ],[
  dnl   -L$CONFIGER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(CONFIGER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(configer, configer.c toml.c, $ext_shared,, -fvisibility=hidden -std=gnu99 -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
