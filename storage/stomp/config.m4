PHP_ARG_ENABLE(jam-stomp, whether to enable jam stomp backend,
[  --enable-jam-stomp     Enable jam stomp backend])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_STOMP" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(jam_stomp, jam_stomp.c jam_stomp_funcs.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_stomp, jam)
fi