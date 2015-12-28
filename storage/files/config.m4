PHP_ARG_ENABLE(jam-files, whether to enable jam files backend,
[  --enable-jam-files     Enable jam files backend])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_FILES" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

  PHP_NEW_EXTENSION(jam_files, jam_files.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_files, jam)
fi