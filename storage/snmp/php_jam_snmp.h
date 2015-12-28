/*
   +----------------------------------------------------------------------+
   | PHP Version 5 / jam                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) 2009 Mikko Koppanen                                    |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Mikko Kopppanen <mkoppanen@php.net>                          |
   +----------------------------------------------------------------------+
*/

#ifndef _PHP_JAM_SNMP_H_
# define _PHP_JAM_SNMP_H_

#define PHP_JAM_SNMP_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "../../php_jam.h"
#include "../../php_jam_storage.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

ZEND_BEGIN_MODULE_GLOBALS(jam_snmp)
	netsnmp_session *snmp_sess;
	char *trap_host;
	char *trap_community;

	char *app_key;

	char *name_oid;
	char *error_msg_oid;
	char *trap_oid;
	char *uuid_oid;
	
	long max_traps;
	long current_hour;
	long sent_traps;
	
	zend_bool enabled;
ZEND_END_MODULE_GLOBALS(jam_snmp)

ZEND_EXTERN_MODULE_GLOBALS(jam_snmp)

#ifdef ZTS
# define JAM_SNMP_G(v) TSRMG(jam_snmp_globals_id, zend_jam_snmp_globals *, v)
#else
# define JAM_SNMP_G(v) (jam_snmp_globals.v)
#endif

/* Hook into jam module */
extern php_jam_storage_module php_jam_storage_module_snmp;
#define php_jam_storage_module_snmp_ptr &php_jam_storage_module_snmp

/* Normal PHP entry */
extern zend_module_entry jam_snmp_module_entry;
#define phpext_jam_snmp_ptr &jam_snmp_module_entry

PHP_JAM_STORAGE_FUNCS(snmp);

#endif
