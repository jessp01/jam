PHP_ARG_ENABLE(jam, whether to enable error monitoring support,
[  --enable-jam     Enable experimental error monitoring support])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM" != "no"; then

  AC_MSG_CHECKING([for libuuid])

  for i in /usr /usr/local /opt /opt/local; do
  	if test -r "$i/include/uuid/uuid.h"; then
  		PHP_JAM_LIBUUID=$i
  		break;
  	fi
  done

  if test -z $PHP_JAM_LIBUUID; then
  	AC_MSG_ERROR([libuuid not found])	
  fi

  AC_MSG_RESULT([$PHP_JAM_LIBUUID])

	if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

  PHP_ADD_INCLUDE("$PHP_JAM_LIBUUID/include")
  PHP_ADD_LIBRARY_WITH_PATH(uuid, "$PHP_JAM_LIBUUID/lib", JAM_SHARED_LIBADD)
  PHP_SUBST(JAM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(jam, jam.c jam_storage.c jam_request.c jam_uuid.c jam_cache.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/jam], [php_jam.h php_jam_storage.h])
  
fi