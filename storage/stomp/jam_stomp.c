

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

#include "php_jam_stomp.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_stomp)

php_jam_storage_module php_jam_storage_module_stomp = {
	PHP_AWARE_STORAGE_MOD(stomp)
};

PHP_AWARE_CONNECT_FUNC(stomp)
{
	char *err_msg;
	int err_code;

	if (!php_jam_stomp_connect(AWARE_STOMP_G(handle), AWARE_STOMP_G(server_uri), 
									AWARE_STOMP_G(username), AWARE_STOMP_G(password), &err_msg, &err_code TSRMLS_CC)) {
		
		if (err_msg) {
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Unable to connect jam_stomp: %s", err_msg);
			efree(err_msg);
		}
		return AwareOperationFailed;
	}
	return AwareOperationSuccess;
}

PHP_AWARE_GET_FUNC(stomp)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_STORE_FUNC(stomp)
{
	smart_str string = {0};
	php_jam_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	if (!php_jam_stomp_send(AWARE_STOMP_G(handle), AWARE_STOMP_G(queue_name), string.c, string.len TSRMLS_CC)) {
		smart_str_free(&string);
		return AwareOperationFailed;
	}
	smart_str_free(&string);
	return AwareOperationSuccess;
}

PHP_AWARE_GET_LIST_FUNC(stomp)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DELETE_FUNC(stomp)
{
	return AwareOperationNotSupported;
}

PHP_AWARE_DISCONNECT_FUNC(stomp)
{
	(void) php_jam_stomp_disconnect(AWARE_STOMP_G(handle) TSRMLS_CC);
	return AwareOperationSuccess;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam_stomp.server_uri", NULL, PHP_INI_PERDIR, OnUpdateString, server_uri, zend_jam_stomp_globals, jam_stomp_globals)
	STD_PHP_INI_ENTRY("jam_stomp.queue_name", "/queue/jam", PHP_INI_PERDIR, OnUpdateString, queue_name, zend_jam_stomp_globals, jam_stomp_globals)
	
	STD_PHP_INI_ENTRY("jam_stomp.username", NULL, PHP_INI_PERDIR, OnUpdateString, username, zend_jam_stomp_globals, jam_stomp_globals)
	STD_PHP_INI_ENTRY("jam_stomp.password", NULL, PHP_INI_PERDIR, OnUpdateString, password, zend_jam_stomp_globals, jam_stomp_globals)
PHP_INI_END()

static void php_jam_stomp_init_globals(zend_jam_stomp_globals *jam_stomp_globals)
{
	jam_stomp_globals->server_uri = NULL;
	jam_stomp_globals->queue_name = NULL;
	
	jam_stomp_globals->username = NULL;
	jam_stomp_globals->password = NULL;
}

/* {{{ PHP_MINIT_FUNCTION(jam_stomp) */
PHP_MINIT_FUNCTION(jam_stomp) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(jam_stomp, php_jam_stomp_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_AWARE_STORAGE_REGISTER(stomp);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:
			if (!AWARE_STOMP_G(server_uri)) {
				AWARE_STOMP_G(enabled) = 0;
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_stomp, missing jam_stomp.server_uri");
				return FAILURE;
			}

			AWARE_STOMP_G(enabled) = 1;
			AWARE_STOMP_G(handle)  = php_jam_stomp_init();
			
			if (!AWARE_STOMP_G(handle))
				php_jam_original_error_cb(E_ERROR TSRMLS_CC, "Failed to allocate memory");
		break;
		
		case AwareModuleFailed:
			AWARE_STOMP_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_STOMP_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_stomp) */
PHP_MSHUTDOWN_FUNCTION(jam_stomp)
{
	UNREGISTER_INI_ENTRIES();
	
	if (AWARE_STOMP_G(enabled) && AWARE_STOMP_G(handle)) {
		php_jam_stomp_deinit(AWARE_STOMP_G(handle) TSRMLS_CC);
	}
	
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_stomp) */
PHP_MINFO_FUNCTION(jam_stomp)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_stomp storage", "enabled");
	php_info_print_table_row(2, "jam_stomp version", PHP_AWARE_STOMP_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_stomp_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_stomp_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_stomp",
        jam_stomp_functions,
        PHP_MINIT(jam_stomp),
        PHP_MSHUTDOWN(jam_stomp),
        NULL,
        NULL,
        PHP_MINFO(jam_stomp),
    	PHP_AWARE_STOMP_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_STOMP
ZEND_GET_MODULE(jam_stomp)
#endif