PHP_ARG_WITH(jam-zeromq2, whether to enable jam zeromq2 backend,
[  --with-jam-zeromq2     Enable jam zeromq2 backend], no)

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_ZEROMQ2" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi
  
  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  if test "x$PKG_CONFIG" = "xno"; then
    AC_MSG_RESULT([pkg-config not found])
    AC_MSG_ERROR([Please reinstall the pkg-config distribution])
  fi

  ORIG_PKG_CONFIG_PATH=$PKG_CONFIG_PATH

  AC_MSG_CHECKING(libzmq installation)
  if test "x$PHP_JAM_ZEROMQ2" = "xyes"; then
    export PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/opt/lib/pkgconfig:/opt/local/lib/pkgconfig
  else
    export PKG_CONFIG_PATH=$PHP_JAM_ZEROMQ2:$PHP_JAM_ZEROMQ2/lib/pkgconfig
  fi

  if $PKG_CONFIG --exists libzmq; then
    PHP_JAM_ZEROMQ2_VERSION=`$PKG_CONFIG libzmq --modversion`

    AC_MSG_RESULT([found version $PHP_JAM_ZEROMQ2_VERSION])
    PHP_JAM_ZEROMQ2_LIBS=`$PKG_CONFIG libzmq --libs`
    PHP_JAM_ZEROMQ2_INCS=`$PKG_CONFIG libzmq --cflags`
    
    PHP_EVAL_LIBLINE($PHP_JAM_ZEROMQ2_LIBS, JAM_ZEROMQ2_SHARED_LIBADD)
    PHP_EVAL_INCLINE($PHP_JAM_ZEROMQ2_INCS)
  else
    AC_MSG_ERROR(Unable to find libzmq installation)
  fi

  PHP_NEW_EXTENSION(jam_zeromq2, jam_zeromq2.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_zeromq2, jam)
  PKG_CONFIG_PATH="$ORIG_PKG_CONFIG_PATH"
  
  PHP_SUBST(JAM_ZEROMQ2_SHARED_LIBADD)
fi