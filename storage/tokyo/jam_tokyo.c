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

#include "php_jam_tokyo.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_tokyo)

php_jam_storage_module php_jam_storage_module_tokyo = {
	PHP_AWARE_STORAGE_MOD(tokyo)
};

PHP_AWARE_CONNECT_FUNC(tokyo)
{
	zend_bool status = 0;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		int mode = TDBOWRITER|TDBOCREAT;
		
		if (!AWARE_TOKYO_G(cabinet_block)) {
			mode |= TDBOLCKNB;
		}
		status = php_jam_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_file), mode);
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_open(AWARE_TOKYO_G(tyrant), AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port));
	}
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_GET_FUNC(tokyo)
{
	zend_bool status = 0;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		status = php_jam_cabinet_get(AWARE_TOKYO_G(cabinet), uuid, event TSRMLS_CC);
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_get(AWARE_TOKYO_G(tyrant), uuid, event TSRMLS_CC);
	}
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_STORE_FUNC(tokyo)
{
	zend_bool status = 0;
	smart_str string = {0};
	
	php_jam_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		status = php_jam_cabinet_put(AWARE_TOKYO_G(cabinet), uuid, string.c, string.len);
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_put(AWARE_TOKYO_G(tyrant), uuid, string.c, string.len);
	}
	smart_str_free(&string);
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_GET_LIST_FUNC(tokyo)
{
	zend_bool status = 0;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		status = php_jam_cabinet_get_list(AWARE_TOKYO_G(cabinet), start, limit, events);
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_get_list(AWARE_TOKYO_G(tyrant), start, limit, events);
	}
	
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_DELETE_FUNC(tokyo)
{
	zend_bool status = 0;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		status = php_jam_cabinet_delete(AWARE_TOKYO_G(cabinet), uuid);
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_delete(AWARE_TOKYO_G(tyrant), uuid);
	}
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

PHP_AWARE_DISCONNECT_FUNC(tokyo)
{
	zend_bool status = 0;
	
	if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
		status = php_jam_cabinet_close(AWARE_TOKYO_G(cabinet));
	} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
		status = php_jam_tyrant_close(AWARE_TOKYO_G(tyrant));
	}
	return (status) ? AwareOperationSuccess : AwareOperationFailed;
}

static PHP_INI_MH(OnUpdateBackend)
{
	if (new_value && new_value_length > 0) {
		if (new_value_length == 6 && !strcmp(new_value, "tyrant")) {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendTyrant;			
		} else if (new_value_length == 7 && !strcmp(new_value, "cabinet")) {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendCabinet;
		} else {
			AWARE_TOKYO_G(backend) = AwareTokyoBackendNotSet;
			return FAILURE;
		}
	} else {
		AWARE_TOKYO_G(backend) = AwareTokyoBackendNotSet;
	}
	return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
}

PHP_INI_BEGIN()
	/* Possible values "cabinet", "tyrant" */
	STD_PHP_INI_ENTRY("jam_tokyo.backend", "", PHP_INI_PERDIR, OnUpdateBackend, backend_str, zend_jam_tokyo_globals, jam_tokyo_globals)
	
	/* Tokyo Tyrant config */
	STD_PHP_INI_ENTRY("jam_tokyo.tyrant_host", "localhost", PHP_INI_PERDIR, OnUpdateString, tyrant_host, zend_jam_tokyo_globals, jam_tokyo_globals)
	STD_PHP_INI_ENTRY("jam_tokyo.tyrant_port", "1978", PHP_INI_PERDIR, OnUpdateLong, tyrant_port, zend_jam_tokyo_globals, jam_tokyo_globals)

	/* Tokyo Cabinet config */
	STD_PHP_INI_ENTRY("jam_tokyo.cabinet_file", "/tmp/casket.tct", PHP_INI_PERDIR, OnUpdateString, cabinet_file, zend_jam_tokyo_globals, jam_tokyo_globals)
	STD_PHP_INI_ENTRY("jam_tokyo.cabinet_block", "1", PHP_INI_PERDIR, OnUpdateBool, cabinet_block, zend_jam_tokyo_globals, jam_tokyo_globals)
	
PHP_INI_END()

static void php_jam_tokyo_init_globals(zend_jam_tokyo_globals *jam_tokyo_globals)
{
	jam_tokyo_globals->backend_str = NULL;
	jam_tokyo_globals->backend     = AwareTokyoBackendNotSet;
	
	/* If backend == tyrant */
	jam_tokyo_globals->tyrant_host = NULL;
	jam_tokyo_globals->tyrant_port = 0;
	
	/* If backend == cabinet */
	jam_tokyo_globals->cabinet_file  = NULL;
	jam_tokyo_globals->cabinet_block = 1;
}

static zend_bool php_jam_tokyo_init_backend(AwareTokyoBackend configured_backend TSRMLS_DC) 
{
	if (configured_backend == AwareTokyoBackendCabinet) {
		int ecode;
		
		AWARE_TOKYO_G(cabinet) = php_jam_cabinet_init();
		
		if (!AWARE_TOKYO_G(cabinet)) {
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to allocate tokyo cabinet handle");
			return 0;
		}
		
		if (!php_jam_cabinet_open(AWARE_TOKYO_G(cabinet), AWARE_TOKYO_G(cabinet_file), TDBOWRITER|TDBOCREAT)) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to open %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;
		}
		
		if (!php_jam_cabinet_optimize(AWARE_TOKYO_G(cabinet))) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to optimize %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;	
		}
		
		if (!php_jam_cabinet_close(AWARE_TOKYO_G(cabinet))) {
			ecode = tctdbecode(AWARE_TOKYO_G(cabinet));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to close %s: %s", AWARE_TOKYO_G(cabinet_file), tctdberrmsg(ecode));
			return 0;
		}
		return 1;
	} else if (configured_backend == AwareTokyoBackendTyrant) {
		int ecode;
		
		AWARE_TOKYO_G(tyrant) = php_jam_tyrant_init();
		
		if (!AWARE_TOKYO_G(tyrant)) {
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to allocate tokyo cabinet handle");
			return 0;
		}
		
		if (!php_jam_tyrant_open(AWARE_TOKYO_G(tyrant), AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to open %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;
		}
		
		if (!php_jam_tyrant_optimize(AWARE_TOKYO_G(tyrant))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to optimize %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;	
		}
		
		if (!php_jam_tyrant_close(AWARE_TOKYO_G(tyrant))) {
			ecode = tcrdbecode(AWARE_TOKYO_G(tyrant));
			php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to close %s:%d: %s", AWARE_TOKYO_G(tyrant_host), AWARE_TOKYO_G(tyrant_port), tcrdberrmsg(ecode));
			return 0;
		}
		return 1;
		
	}
	return 0;
}

/* {{{ PHP_MINIT_FUNCTION(jam_tokyo) */
PHP_MINIT_FUNCTION(jam_tokyo) 
{
	AwareModuleRegisterStatus status;
	
	ZEND_INIT_MODULE_GLOBALS(jam_tokyo, php_jam_tokyo_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	status = PHP_AWARE_STORAGE_REGISTER(tokyo);
	
	switch (status) 
	{
		case AwareModuleRegistered:	
			if (AWARE_TOKYO_G(backend) == AwareTokyoBackendNotSet) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_tokyo, no jam_tokyo.backend defined");
				return FAILURE;
			}
			if (!php_jam_tokyo_init_backend(AWARE_TOKYO_G(backend) TSRMLS_CC)) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Failed to initialize the tokyo backend");
				return FAILURE;
			}
			AWARE_TOKYO_G(enabled) = 1;
		break;
		
		case AwareModuleFailed:
			AWARE_TOKYO_G(enabled) = 0;
			return FAILURE;
		break;

		case AwareModuleNotConfigured:
			AWARE_TOKYO_G(enabled) = 0;
		break;	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_tokyo) */
PHP_MSHUTDOWN_FUNCTION(jam_tokyo)
{
	UNREGISTER_INI_ENTRIES();
	
	if (AWARE_TOKYO_G(enabled)) {
		if (AWARE_TOKYO_G(backend) == AwareTokyoBackendCabinet) {
			php_jam_cabinet_deinit(AWARE_TOKYO_G(cabinet));
		} else if (AWARE_TOKYO_G(backend) == AwareTokyoBackendTyrant) {
			php_jam_tyrant_deinit(AWARE_TOKYO_G(tyrant));
		}
	}
	
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_tokyo) */
PHP_MINFO_FUNCTION(jam_tokyo)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_tokyo storage", "enabled");
	php_info_print_table_row(2, "jam_tokyo version", PHP_AWARE_TOKYO_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_tokyo_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_tokyo_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_tokyo",
        jam_tokyo_functions,
        PHP_MINIT(jam_tokyo),
        PHP_MSHUTDOWN(jam_tokyo),
        NULL,
        NULL,
        PHP_MINFO(jam_tokyo),
    	PHP_AWARE_TOKYO_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AWARE_TOKYO
ZEND_GET_MODULE(jam_tokyo)
#endif