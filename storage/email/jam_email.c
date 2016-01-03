/*
   +----------------------------------------------------------------------+
   | PHP Version 5 / jam                                                |
   +----------------------------------------------------------------------+
   | Copyright (c) Mikko Koppanen, Jess Portnoy                           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: 								  |
   |	Mikko Kopppanen <mkoppanen@php.net>                               |
   |	Jess Portnoy <jess.portnoy@kaltura.com>                           |
   +----------------------------------------------------------------------+
*/

#include "php_jam_email.h"

#include "main/php_output.h"
#include "ext/standard/php_var.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_email)

php_jam_storage_module php_jam_storage_module_email = {
	PHP_JAM_STORAGE_MOD(email)
};

PHP_JAM_CONNECT_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_JAM_GET_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_JAM_STORE_FUNC(email)
{
	zval fname, argies[3], retval, *ppzval;

	/*
		Error body
	*/
	//MAKE_STD_ZVAL(argies[2]);
#if ZEND_MODULE_API_NO <= PHP_5_3_X_API_NO
	php_start_ob_buffer(NULL, 4096, 0 TSRMLS_CC);
#else
	php_output_start_user(NULL, 4096, PHP_OUTPUT_HANDLER_STDFLAGS);
#endif
	php_var_dump(event, JAM_G(depth) TSRMLS_CC);

#if ZEND_MODULE_API_NO <= PHP_5_3_X_API_NO
	if (php_ob_get_buffer(argies[2] TSRMLS_CC) == FAILURE) {
#else
	if (php_output_get_contents(&argies[2] TSRMLS_CC) == FAILURE) {
#endif
		//zval_dtor(argies[2]);
		//FREE_ZVAL(argies[2]);
		
#if ZEND_MODULE_API_NO <= PHP_5_3_X_API_NO
		php_end_ob_buffer(0, 0 TSRMLS_CC);
#else
		php_output_end();
#endif
		
		return AwareOperationFailed;
    }
#if ZEND_MODULE_API_NO <= PHP_5_3_X_API_NO
	php_end_ob_buffer(0, 0 TSRMLS_CC);
#else
		php_output_end();
#endif

	//MAKE_STD_ZVAL(fname);
	ZVAL_STRING(&fname, "mail");

	/*
		Recipient
	*/
	//MAKE_STD_ZVAL(argies[0]);
	ZVAL_STRING(&argies[0], JAM_EMAIL_G(to_address));
	//ZVAL_COPY_VALUE(&argies[0], &JAM_EMAIL_G(to_address));

	/*
		Subject
	*/
	//MAKE_STD_ZVAL(argies[1]);

	//if (zend_hash_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message"), (void **) &ppzval) == SUCCESS) {
	if ((ppzval = zend_hash_str_find(Z_ARRVAL_P(event), "error_message", sizeof("error_message")-1)) != NULL) {
		ZVAL_STRING(&argies[1], Z_STRVAL_P(ppzval));
		//ZVAL_COPY_VALUE(&argies[1], Z_STRVAL_P(ppzval));
	} else {
		ZVAL_STRING(&argies[1], "Aware: No error message");
	}

	//MAKE_STD_ZVAL(retval);
	call_user_function(EG(function_table), NULL, &fname, &retval, 3, argies);

	zval_dtor(&fname);
	//FREE_ZVAL(&fname);

	zval_dtor(&retval);
	//FREE_ZVAL(&retval);

	zval_dtor(&argies[0]);
	//FREE_ZVAL(&argies[0]);

	zval_dtor(&argies[1]);
	//FREE_ZVAL(&argies[1]);

	zval_dtor(&argies[2]);
	//FREE_ZVAL(&argies[2]);
	
	return AwareOperationSuccess;
}

PHP_JAM_GET_LIST_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DELETE_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_JAM_DISCONNECT_FUNC(email)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam_email.to_address", NULL, PHP_INI_PERDIR, OnUpdateString, to_address, zend_jam_email_globals, jam_email_globals)
PHP_INI_END()

static void php_jam_email_init_globals(zend_jam_email_globals *jam_email_globals)
{
	jam_email_globals->to_address = NULL;
}

/* {{{ PHP_MINIT_FUNCTION(jam_email) */
PHP_MINIT_FUNCTION(jam_email) 
{
	AwareModuleRegisterStatus reg_status;
	
	ZEND_INIT_MODULE_GLOBALS(jam_email, php_jam_email_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_JAM_STORAGE_REGISTER(email);
	
	switch (reg_status) 
	{
		case AwareModuleRegistered:
			if (!JAM_EMAIL_G(to_address)) {
				php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_email, missing jam_email.to_address");
				return FAILURE;
			}
		break;
		
		case AwareModuleFailed:
			return FAILURE;
		break;
		
		case AwareModuleNotConfigured:
		break;
	}
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_email) */
PHP_MSHUTDOWN_FUNCTION(jam_email)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_email) */
PHP_MINFO_FUNCTION(jam_email)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_email storage", "enabled");
	php_info_print_table_row(2, "jam_email version", PHP_JAM_EMAIL_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_email_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_email_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_email",
        jam_email_functions,
        PHP_MINIT(jam_email),
        PHP_MSHUTDOWN(jam_email),
        NULL,
        NULL,
        PHP_MINFO(jam_email),
    	PHP_JAM_EMAIL_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_JAM_EMAIL
ZEND_GET_MODULE(jam_email)
#endif
