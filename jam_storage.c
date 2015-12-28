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

#include "php_jam_private.h"
#include "php_jam_storage.h"

#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_string.h"

ZEND_DECLARE_MODULE_GLOBALS(jam)

/*
	MAX_MODULES defines how many active modules there can be any given time.
	'10 should be more than enough for everybody..'
*/
#define MAX_MODULES 10

/*
	The module structure follows session module structure quite closely
*/
static php_jam_storage_module *php_jam_storage_modules[MAX_MODULES + 1] = { 0 };

/* {{{ static zend_bool php_jam_storage_module_is_configured(const char *mod_name TSRMLS_DC) 
*/
static zend_bool php_jam_storage_module_is_configured(const char *mod_name TSRMLS_DC) 
{
	zend_bool retval = 0;
	char *pch, *last, *ptr;
	
	/* If jam is not enabled, don't register anything */
	if (!AWARE_G(enabled)) {
		return 0;
	}

	ptr = estrdup(AWARE_G(storage_modules));
	pch = php_strtok_r(ptr, ",", &last);

	while (pch != NULL) {
		char *mod = php_trim(pch, strlen(pch), NULL, 0, NULL, 3 TSRMLS_CC);
		
		if (mod) {
			if (!strcmp(mod, mod_name)) {
				retval = 1;
			}
			efree(mod);
		}
		
		/* all done ? */
		if (retval)
			break;
		
		pch = php_strtok_r(NULL, ",", &last);
	}
	
	efree(ptr);
	return retval;
}
/* }}} */

/* {{{ MY_AWARE_EXPORTS AwareModuleRegisterStatus php_jam_register_storage_module(php_jam_storage_module *mod TSRMLS_DC)
*/
MY_AWARE_EXPORTS AwareModuleRegisterStatus php_jam_register_storage_module(php_jam_storage_module *mod TSRMLS_DC)
{
	int i, ret = AwareModuleFailed;
	
	jam_printf("Registering storage module: %s\n", mod->name);
	
	if (!php_jam_storage_module_is_configured(mod->name TSRMLS_CC)) {
		jam_printf("Storage module %s is not configured\n", mod->name);
		return AwareModuleNotConfigured;
	}

	for (i = 0; i < MAX_MODULES; i++) {
		if (!php_jam_storage_modules[i]) {
			php_jam_storage_modules[i] = mod;
			ret = AwareModuleRegistered;
			break;
		}
	}
	return ret;	
}
/* }}} */

/* {{{ php_jam_storage_module *php_jam_find_storage_module(const char *mod_name) 
*/
php_jam_storage_module *php_jam_find_storage_module(const char *mod_name) 
{
	int i;
	php_jam_storage_module *ret = NULL;
	
	jam_printf("Finding storage module %s\n", mod_name);
	
	for (i = 0; i < MAX_MODULES; i++) {
		if (php_jam_storage_modules[i] && !strcmp(mod_name, php_jam_storage_modules[i]->name)) {
			jam_printf("Found storage module %s\n", mod_name);
			ret = php_jam_storage_modules[i];
			break;
		}
	}
	return ret;
}
/* }}} */

/* {{{ void php_jam_storage_module_list(zval *return_value)
	* return_value must be initialized as an array before calling this function
*/
void php_jam_storage_module_list(zval *return_value) 
{
	int i;

	for (i = 0; i < MAX_MODULES; i++) {
		if (php_jam_storage_modules[i]) {
			add_next_index_string(return_value, php_jam_storage_modules[i]->name, 1);
		}
	}
}
/* }}} */

/* {{{ MY_AWARE_EXPORTS void php_jam_storage_serialize(const char *uuid, zval *event, smart_str *data_var TSRMLS_DC)
*/
MY_AWARE_EXPORTS void php_jam_storage_serialize(const char *uuid, zval *event, smart_str *data_var TSRMLS_DC)
{
	php_serialize_data_t var_hash;
	
	if (AWARE_G(use_cache)) {
		if (php_jam_cache_get(&AWARE_G(s_cache), uuid, data_var)) {
			return;
		}
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(data_var, &event, &var_hash TSRMLS_CC);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (AWARE_G(use_cache)) {
		php_jam_cache_store(&AWARE_G(s_cache), uuid, data_var);
	}
}
/* }}} */

/* {{{ MY_AWARE_EXPORTS zend_bool php_jam_storage_unserialize(const char *string, int string_len, zval *retval TSRMLS_DC)
*/
MY_AWARE_EXPORTS zend_bool php_jam_storage_unserialize(const char *string, int string_len, zval *retval TSRMLS_DC)
{
	zend_bool unserialized;
	php_unserialize_data_t var_hash;
	const unsigned char *p, *s;

	p = s = (const unsigned char *)string;
	
	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	unserialized = php_var_unserialize(&retval, (const unsigned char **)&p, (const unsigned char *) s + string_len, &var_hash TSRMLS_CC);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	
	return unserialized;
}

/* Sends the event to storage module */
void php_jam_storage_store(php_jam_storage_module *mod, const char *uuid, zval *event, long type, const char *error_filename, long error_lineno TSRMLS_DC) 
{
	jam_printf("Storing event to module: %s\n", mod->name);
	
	/*
		User can override the storage module error reporting per module basis
		This means that we need to check here if the module is configured to 
		store events of this level. 
	*/
	if (zend_hash_num_elements(&AWARE_G(module_error_reporting)) > 0) {
		long **level;
		/* This means that we might have overriden error reporting level */
		if (zend_hash_find(&AWARE_G(module_error_reporting), mod->name, strlen(mod->name) + 1, (void **)&level) == SUCCESS) {
			/* Check if module is configured for this sort of errors */
			if (!(**level & type)) {
				return;
			}
		}
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod->name);
		return;
	}
	
	if (mod->store(uuid, event, error_filename, error_lineno TSRMLS_CC,type,AWARE_G(appname),AWARE_G(source_baseurl)) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to store the event %s (%s)", uuid, mod->name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}

/* Sends the event to all available storage modules */
void php_jam_storage_store_all(const char *uuid, zval *event, long type, const char *error_filename, long error_lineno TSRMLS_DC) 
{
	int i;

	for (i = 0; i < MAX_MODULES; i++) {
		if (php_jam_storage_modules[i]) {
			php_jam_storage_store(php_jam_storage_modules[i], uuid, event, type, error_filename, error_lineno TSRMLS_CC);
		}
	}
}

/* get the event from storage modules */
void php_jam_storage_get(const char *mod_name, const char *uuid, zval *return_value TSRMLS_DC) 
{
	php_jam_storage_module *mod;
	mod = php_jam_find_storage_module(mod_name);

	if (!mod) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Storage module(%s) does not exist", mod_name);
		return;
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod_name);
		return;
	}

	if (mod->get(uuid, return_value TSRMLS_CC) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to get the event %s (%s)", uuid, mod_name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}

/* get the event from storage modules */
void php_jam_storage_get_list(const char *mod_name, long start, long limit, zval *return_value TSRMLS_DC) 
{
	php_jam_storage_module *mod;
	mod = php_jam_find_storage_module(mod_name);

	if (!mod) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Storage module(%s) does not exist", mod_name);
		return;
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod_name);
		return;
	}
	
	if (mod->get_list(start, limit, return_value TSRMLS_CC) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to get event list (%s)", mod_name);
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
	}
}

/* get the event from storage modules */
zend_bool php_jam_storage_delete(const char *mod_name, const char *uuid TSRMLS_DC) 
{
	zend_bool status = 1;
	
	php_jam_storage_module *mod;
	mod = php_jam_find_storage_module(mod_name);

	if (!mod) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Unable to find storage module(%s)", mod_name);
		return 0;
	}
	
	/* Connect failed, report error and bail out */
	if (mod->connect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to connect the storage module (%s)", mod_name);
		return 0;
	}
	
	if (mod->delete(uuid TSRMLS_CC) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to delete the event %s (%s)", uuid, mod_name);
		status = 0;
	}

	if (mod->disconnect(TSRMLS_C) == AwareOperationFailed) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to disconnect storage module (%s)", mod->name);
		status = 0;
	}
	
	return status;
}
