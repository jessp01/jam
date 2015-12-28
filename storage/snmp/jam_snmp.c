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

#include "php_jam_snmp.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_snmp)

/* Required oids for traps */
static const oid objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
static const oid objid_snmptrap[]  = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };

/* {{{ AwareOperationStatus php_jam_send_snmp_trap(netsnmp_session *sess, const char *uuid, const char *message, const char *error_filename, long error_lineno TSRMLS_DC) 
*/
AwareOperationStatus php_jam_send_snmp_trap(netsnmp_session *sess, const char *uuid, const char *message, const char *error_filename, long error_lineno TSRMLS_DC) 
{
	size_t oid_len = MAX_OID_LEN;
	
	oid objid_uuid[MAX_OID_LEN];
	oid objid_php_name[MAX_OID_LEN];
	oid objid_php_message[MAX_OID_LEN];
	
	/* Required for the trap */
	char csysuptime[20];
	char *trap = NULL, *error_file_line;
	long sysuptime;
	int retval;
	
	netsnmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
	
	if (!pdu) {
		return FAILURE;
	}
	
	/* For some reason sysuptime is needed in a trap */
	sysuptime = get_uptime();
	memset(csysuptime, 0, 20);
	sprintf(csysuptime, "%ld", sysuptime);
	trap = csysuptime;
	
	if (snmp_add_var(pdu, objid_sysuptime, sizeof(objid_sysuptime) / sizeof(oid), 't', trap) != 0) {
		return AwareOperationFailed;
	}

	if (snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid), 'o', JAM_SNMP_G(error_msg_oid)) != 0) {
		return AwareOperationFailed;
	}
	
	/* First parameter is the uuid of the event */
	if (!snmp_parse_oid(JAM_SNMP_G(uuid_oid), objid_uuid, &oid_len)) {
		return AwareOperationFailed;
	}
	if (snmp_add_var(pdu, objid_uuid, oid_len, 's', uuid) != 0) {
		return AwareOperationFailed;
	}

	/* Next filename and line */
	if (!snmp_parse_oid(JAM_SNMP_G(name_oid), objid_php_name, &oid_len)) {
		return AwareOperationFailed;
	}

	/* The script filename and line */
	spprintf(&error_file_line, MAXPATHLEN + 256, "%s:%ld", error_filename, error_lineno);
	retval = snmp_add_var(pdu, objid_php_name, oid_len, 's', error_file_line);
	efree(error_file_line);
	
	if (retval != 0) {
		return AwareOperationFailed;
	}

	/* And error message last */
	if (!snmp_parse_oid(JAM_SNMP_G(error_msg_oid), objid_php_message, &oid_len)) {
		return AwareOperationFailed;
	}
	if (snmp_add_var(pdu, objid_php_message, oid_len, 's', message) != 0) {
		return AwareOperationFailed;
	}

	/* Send the trap */
	if (snmp_send(sess, pdu) == 0) {
		return AwareOperationFailed;
	}

	return AwareOperationSuccess;
}
/* }}} */

/* {{{ 
*/
netsnmp_session *php_jam_snmp_init_snmp_session(char *host, char *community) 
{
	netsnmp_session session, *ss;
	
	init_snmp("php_jam_snmp");
	SOCK_STARTUP;	
	
	/* prevents the configuration persisting */
	netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_PERSIST_STATE, 1);	
	
	snmp_sess_init(&session);
	
	session.peername = host;
	session.version = SNMP_VERSION_2c;
	session.community = (u_char *)community;
	session.community_len = strlen(community);
	
	ss = snmp_open(&session);
	
	if (ss == NULL) {
		SOCK_CLEANUP;
		snmp_shutdown("php_jam_snmp");
		return NULL;
	}	
	return ss;
}
/* }}} */

/* {{{ 
*/
void php_jam_deinit_snmp_session(netsnmp_session *session) 
{
	if (session) {
		snmp_close(session);
	}

	snmp_shutdown("php_jam_snmp");
	SOCK_CLEANUP;
}
/* }}} */

php_jam_storage_module php_jam_storage_module_snmp = {
	PHP_JAM_STORAGE_MOD(snmp)
};

PHP_JAM_CONNECT_FUNC(snmp)
{
	/* First time connect */
	if (!JAM_SNMP_G(snmp_sess)) {
		JAM_SNMP_G(snmp_sess) = php_jam_snmp_init_snmp_session(JAM_SNMP_G(trap_host), JAM_SNMP_G(trap_community));
		
		if (!JAM_SNMP_G(snmp_sess)) {
			return AwareOperationFailed;
		}
	}
	return AwareOperationSuccess;
}

PHP_JAM_GET_FUNC(snmp)
{
	return AwareOperationNotSupported;
}

PHP_JAM_STORE_FUNC(snmp)
{
	zval **ppzval;
	if (zend_hash_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message"), (void **) &ppzval) == SUCCESS) {
		return php_jam_send_snmp_trap(JAM_SNMP_G(snmp_sess), uuid, Z_STRVAL_PP(ppzval), error_filename, error_lineno TSRMLS_CC);
	} else {
		return php_jam_send_snmp_trap(JAM_SNMP_G(snmp_sess), uuid, "No error message", error_filename, error_lineno TSRMLS_CC);
	}
}

PHP_JAM_GET_LIST_FUNC(snmp)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DELETE_FUNC(snmp)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DISCONNECT_FUNC(snmp)
{
	/* Disconnect happens on MSHUTDOWN */
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam_snmp.trap_host", NULL, PHP_INI_SYSTEM, OnUpdateString, trap_host, zend_jam_snmp_globals, jam_snmp_globals)
	STD_PHP_INI_ENTRY("jam_snmp.trap_community", NULL, PHP_INI_PERDIR, OnUpdateString, trap_community, zend_jam_snmp_globals, jam_snmp_globals)
	STD_PHP_INI_ENTRY("jam_snmp.name_oid", NULL,  PHP_INI_PERDIR, OnUpdateString, name_oid, zend_jam_snmp_globals, jam_snmp_globals)
	STD_PHP_INI_ENTRY("jam_snmp.error_msg_oid", NULL, PHP_INI_PERDIR, OnUpdateString, error_msg_oid, zend_jam_snmp_globals, jam_snmp_globals)
	STD_PHP_INI_ENTRY("jam_snmp.trap_oid", NULL, PHP_INI_PERDIR, OnUpdateString, trap_oid, zend_jam_snmp_globals, jam_snmp_globals)
	STD_PHP_INI_ENTRY("jam_snmp.uuid_oid", NULL, PHP_INI_PERDIR, OnUpdateString, uuid_oid, zend_jam_snmp_globals, jam_snmp_globals)
PHP_INI_END()

static void php_jam_snmp_init_globals(zend_jam_snmp_globals *jam_snmp_globals)
{
	/* Set the snmp session to null */
	jam_snmp_globals->snmp_sess = NULL;
	
	/* snmp specific vars */
	jam_snmp_globals->trap_host = NULL;
	jam_snmp_globals->trap_community = NULL;
	
	/* oids */
	jam_snmp_globals->trap_oid = NULL;
	jam_snmp_globals->name_oid = NULL;
	jam_snmp_globals->error_msg_oid = NULL;
	jam_snmp_globals->uuid_oid = NULL;
}

static zend_bool php_jam_snmp_check_config(TSRMLS_D)
{
	if (!JAM_SNMP_G(trap_host)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.trap_host");
		return 0;
	}

	if (!JAM_SNMP_G(trap_community)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.trap_community");
		return 0;
	}

	if (!JAM_SNMP_G(name_oid)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.name_oid");
		return 0;
	}

	if (!JAM_SNMP_G(error_msg_oid)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.error_msg_oid");
		return 0;
	}
	
	if (!JAM_SNMP_G(uuid_oid)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.uuid_oid");
		return 0;
	}

	if (!JAM_SNMP_G(trap_oid)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_snmp, missing jam_snmp.trap_oid");
		return 0;
	}
	return 1;
}

/* {{{ PHP_MINIT_FUNCTION(jam_snmp) */
PHP_MINIT_FUNCTION(jam_snmp) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(jam_snmp, php_jam_snmp_init_globals, NULL);
	REGISTER_INI_ENTRIES();

    reg_status = PHP_JAM_STORAGE_REGISTER(snmp);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:
			if (!php_jam_snmp_check_config(TSRMLS_C)) {
				JAM_SNMP_G(enabled) = 0;
				return FAILURE;
			}
			JAM_SNMP_G(enabled) = 1;
		break;
		
		case AwareModuleFailed:
			JAM_SNMP_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			JAM_SNMP_G(enabled) = 0;
		break;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_snmp) */
PHP_MSHUTDOWN_FUNCTION(jam_snmp)
{
	if (JAM_SNMP_G(enabled)) {
		if (JAM_SNMP_G(snmp_sess)) {
			php_jam_deinit_snmp_session(JAM_SNMP_G(snmp_sess));
		}
	}
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_snmp) */
PHP_MINFO_FUNCTION(jam_snmp)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_snmp storage", "enabled");
	php_info_print_table_row(2, "jam_snmp version", PHP_JAM_SNMP_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_snmp_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_snmp_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_snmp",
        jam_snmp_functions,
        PHP_MINIT(jam_snmp),
        PHP_MSHUTDOWN(jam_snmp),
        NULL,
        NULL,
        PHP_MINFO(jam_snmp),
    	PHP_JAM_SNMP_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_JAM_SNMP
ZEND_GET_MODULE(jam_snmp)
#endif