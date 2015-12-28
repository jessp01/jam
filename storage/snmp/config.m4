PHP_ARG_WITH(jam-snmp,    whether to enable jam snmp extension,
[  --with-jam-snmp[=DIR]     Enable jam snmp extension. DIR is the path to net-snmp-config])

PHP_ARG_ENABLE(jam-debug, whether to enable debugging,
[  --enable-jam-debug     Enable debugging], no, no)

if test "$PHP_JAM_SNMP" != "no"; then

	AC_MSG_CHECKING(net-snmp-config program)
	
	for i in $PHP_JAM_SNMP $PHP_JAM_SNMP/bin /usr/bin /usr/local/bin; do
		if test -f $i/net-snmp-config; then
			NET_SNMP_CONF=$i/net-snmp-config
			AC_MSG_RESULT(found in $NET_SNMP_CONF)
		fi
	done
	
	if test -z $NET_SNMP_CONF; then
		AC_MSG_ERROR(not found)
	fi

	if test ! -x $NET_SNMP_CONF; then
		AC_MSG_ERROR($NET_SNMP_CONF is not executable)
	fi
	
	if test "$PHP_JAM_DEBUG" != "no"; then
    AC_DEFINE([_JAM_DEBUG_], 1, [Enable debugging])
  fi

	PHP_JAM_SNMP_INCS="$CFLAGS `$NET_SNMP_CONF --cflags`"
	PHP_JAM_SNMP_LIBS="$LDFLAGS `$NET_SNMP_CONF --netsnmp-libs`"
	
	PHP_EVAL_LIBLINE($PHP_JAM_SNMP_LIBS, JAM_SNMP_SHARED_LIBADD)
    PHP_EVAL_INCLINE($PHP_JAM_SNMP_INCS)

	PHP_NEW_EXTENSION(jam_snmp, jam_snmp.c, $ext_shared)
	PHP_ADD_EXTENSION_DEP(jam_snmp, jam)
	PHP_SUBST(JAM_SNMP_SHARED_LIBADD)
fi