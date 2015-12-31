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

#include "php_jam_spread.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_spread)

php_jam_storage_module php_jam_storage_module_spread = {
	PHP_JAM_STORAGE_MOD(spread)
};

PHP_JAM_CONNECT_FUNC(spread)
{	
	if (!JAM_SPREAD_G(connected)) {
		int retval;
		char private_group[MAX_GROUP_NAME];
		
		retval = SP_connect(JAM_SPREAD_G(spread_name), JAM_SPREAD_G(user_name), 0, 1, &JAM_SPREAD_G(spread_mailbox), private_group);
	
		if (retval < 0) {
			return AwareOperationFailed;
		}
		JAM_SPREAD_G(connected) = 1;
	}
	
	return AwareOperationSuccess;
}

PHP_JAM_GET_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_JAM_STORE_FUNC(spread)
{
	int retval;
	smart_string string = {0};
	
	php_jam_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	retval = SP_multicast(JAM_SPREAD_G(spread_mailbox), AGREED_MESS, JAM_SPREAD_G(group_name), 1, string.len, string.c);
	smart_str_free(&string);
	
	return (retval < 0) ? AwareOperationFailed : AwareOperationSuccess;
}

PHP_JAM_GET_LIST_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DELETE_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DISCONNECT_FUNC(spread)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam_spread.spread_name",	NULL, PHP_INI_PERDIR, OnUpdateString, spread_name, zend_jam_spread_globals, jam_spread_globals)
	STD_PHP_INI_ENTRY("jam_spread.group_name",	NULL, PHP_INI_PERDIR, OnUpdateString, group_name, zend_jam_spread_globals, jam_spread_globals)
	STD_PHP_INI_ENTRY("jam_spread.user_name",		NULL, PHP_INI_PERDIR, OnUpdateString, user_name, zend_jam_spread_globals, jam_spread_globals)
PHP_INI_END()

static void php_jam_spread_init_globals(zend_jam_spread_globals *jam_spread_globals)
{
	jam_spread_globals->spread_name = NULL;
	jam_spread_globals->group_name  = NULL;
	jam_spread_globals->user_name   = NULL;
	
	jam_spread_globals->connected   = 0;
}

/* {{{ PHP_MINIT_FUNCTION(jam_spread) */
PHP_MINIT_FUNCTION(jam_spread) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(jam_spread, php_jam_spread_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_JAM_STORAGE_REGISTER(spread);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:	
			JAM_SPREAD_G(enabled) = 1;
			
			if (!JAM_SPREAD_G(spread_name)) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_spread, missing jam_spread.spread_name");
				return FAILURE;
			}
			
			if (!JAM_SPREAD_G(group_name)) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_spread, missing jam_spread.group_name");
				return FAILURE;
			}
			
			if (!JAM_SPREAD_G(user_name)) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_spread, missing jam_spread.user_name");
				return FAILURE;
			}

		break;
		
		case AwareModuleFailed:
			JAM_SPREAD_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			JAM_SPREAD_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_spread) */
PHP_MSHUTDOWN_FUNCTION(jam_spread)
{
	UNREGISTER_INI_ENTRIES();
	
	if (JAM_SPREAD_G(connected)) {
		SP_disconnect(JAM_SPREAD_G(spread_mailbox));
		JAM_SPREAD_G(connected) = 0;
	}
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_spread) */
PHP_MINFO_FUNCTION(jam_spread)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_spread storage", "enabled");
	php_info_print_table_row(2, "jam_spread version", PHP_JAM_SPREAD_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_spread_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_spread_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_spread",
        jam_spread_functions,
        PHP_MINIT(jam_spread),
        PHP_MSHUTDOWN(jam_spread),
        NULL,
        NULL,
        PHP_MINFO(jam_spread),
    	PHP_JAM_SPREAD_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_JAM_SPREAD
ZEND_GET_MODULE(jam_spread)
#endif
