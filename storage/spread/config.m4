PHP_ARG_WITH(jam-spread=[DIR], whether to enable jam spread backend,
[  --with-jam-spread=[DIR]     Enable jam spread backend])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_SPREAD" != "no"; then

  if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi
  
  if test -r "$PHP_JAM_SPREAD/include/sp.h"; then
		PHP_JAM_SPREAD_DIR="$PHP_JAM_SPREAD"
	else
		AC_MSG_CHECKING(for libspread in default path)
		for i in /usr /usr/local; do
			if test -r "$i/include/sp.h"; then
				PHP_JAM_SPREAD_DIR=$i
				AC_MSG_RESULT(found in $i)
			fi
		done
	fi
	
	PHP_ADD_INCLUDE($PHP_JAM_SPREAD_DIR/include)

  PHP_ADD_LIBRARY_WITH_PATH(spread, $PHP_JAM_SPREAD_DIR/$PHP_LIBDIR, JAM_SPREAD_SHARED_LIBADD)
  AC_DEFINE(HAVE_JAM_SPREAD,1,[ ])

  PHP_NEW_EXTENSION(jam_spread, jam_spread.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(jam_spread, jam)
  
  PHP_SUBST(JAM_SPREAD_SHARED_LIBADD)
  
fi