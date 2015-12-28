PHP_ARG_ENABLE(jam-email, whether to enable jam email backend,
[  --enable-jam-email     Enable jam email backend])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_EMAIL" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(jam_email, jam_email.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_email, jam)
fi