PHP_ARG_ENABLE(jam-skeleton, whether to enable jam skeleton backend,
[  --enable-jam-skeleton     Enable jam skeleton backend])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_SKELETON" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(jam_skeleton, jam_skeleton.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_skeleton, jam)
fi