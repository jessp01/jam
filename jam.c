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

#include "php_jam_private.h"
#include "php_jam.h"
#include "Zend/zend_builtin_functions.h"
#include "ext/standard/php_string.h"

#include <fcntl.h>
#include <sys/mman.h>

#include "zend.h"
#include "zend_ini_scanner.h"
#include "zend_language_scanner.h"
#include <zend_language_parser.h>

ZEND_DECLARE_MODULE_GLOBALS(jam)

static void php_jam_user_event_trigger(int type TSRMLS_DC, const char *error_filename, const uint error_lineno, const char *format, ...);

/* {{{ jam_event_trigger(int error_level, string message)
	Trigger an event
*/
PHP_FUNCTION(jam_event_trigger)
{
	char *message;
	int message_len;
	const char *error_filename;
	int error_lineno = 0;
	long type; 

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &type, &message, &message_len) != SUCCESS) {
		return;
	}
	
	if (!AWARE_G(enable_event_trigger))
		RETURN_FALSE;

	error_filename = zend_get_executed_filename(TSRMLS_C);
	error_lineno   = zend_get_executed_lineno(TSRMLS_C);
	
	if (type & AWARE_G(log_level)) {
		php_jam_user_event_trigger(type TSRMLS_CC, error_filename, error_lineno, message);
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ jam_event_get(string module_name, string uuid)
	Get event as an array
*/
PHP_FUNCTION(jam_event_get)
{
	char *uuid, *mod_name;
	int uuid_len, mod_name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &mod_name, &mod_name_len, &uuid, &uuid_len) != SUCCESS) {
		return;
	}
	php_jam_storage_get(mod_name, uuid, return_value TSRMLS_CC);
	return;
}
/* }}} */

/* {{{ jam_event_get_list(string module_name[ ,int start, int limit])
	Get list of events
*/
PHP_FUNCTION(jam_event_get_list)
{
	char *mod_name;
	int mod_name_len;
	long start = 0, limit = 10;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &mod_name, &mod_name_len, &start, &limit) != SUCCESS) {
		return;
	}
	
	php_jam_storage_get_list(mod_name, start, limit, return_value TSRMLS_CC);
	return;
}
/* }}} */

/* {{{ jam_event_delete(string module_name, string uuid)
	Delete the event
*/
PHP_FUNCTION(jam_event_delete)
{
	char *uuid, *mod_name;
	int uuid_len, mod_name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &mod_name, &mod_name_len, &uuid, &uuid_len) != SUCCESS) {
		return;
	}
	
	if (php_jam_storage_delete(mod_name, uuid TSRMLS_CC) == AwareOperationSuccess) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ jam_storage_module_list()
	Get list of active storage modules
*/
PHP_FUNCTION(jam_storage_module_list)
{
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	array_init(return_value);
	php_jam_storage_module_list(return_value);

	return;
}
/* }}} */

/* {{{ __jam_error_handler_callback
	Callback
*/
PHP_FUNCTION(__jam_error_handler_callback)
{
	if (AWARE_G(user_error_handler)) {
		zval *args[5], retval;
	
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|zzz", &args[0], &args[1], &args[2], &args[3], &args[4]) != SUCCESS) {
			return;
		}
		
		jam_printf("Invoking error handler: %s\n", Z_STRVAL_P(AWARE_G(user_error_handler)));
		php_jam_invoke_handler(Z_LVAL_P(args[0]) TSRMLS_CC, Z_STRVAL_P(args[2]), Z_LVAL_P(args[3]), Z_STRVAL_P(args[1]));
		call_user_function(EG(function_table), NULL, AWARE_G(user_error_handler), &retval, 5, args TSRMLS_CC);
	}
}
/* }}} */

PHP_FUNCTION(jam_set_error_handler)
{
	if (AWARE_G(orig_set_error_handler)) {
		AWARE_G(orig_set_error_handler)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

		/* Take the user error handler and push to our stack */
		if (EG(user_error_handler)) {
			zval *tmp;
			
			/* Override the error handler with our callback */
			if (!strcmp(Z_STRVAL_P(EG(user_error_handler)), "__jam_error_handler_callback")) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Do not call set_error_handler(\"__jam_error_handler_callback\")");
			}
			
			if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
				zval *old_handler = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
				zend_ptr_stack_push(&AWARE_G(user_error_handlers), old_handler);
			
				zval_dtor(return_value);
				ZVAL_STRING(return_value, Z_STRVAL_P(old_handler), 1);
			}
			
			ALLOC_INIT_ZVAL(tmp);
			ZVAL_STRING(tmp, Z_STRVAL_P(EG(user_error_handler)), 1);

			/* free previous error handler */
			if (AWARE_G(user_error_handler)) {
				//zval_ptr_dtor(&AWARE_G(user_error_handler));
				//FREE_ZVAL(AWARE_G(user_error_handler));
			}
			
			ALLOC_INIT_ZVAL(AWARE_G(user_error_handler));
			ZVAL_STRING(AWARE_G(user_error_handler), Z_STRVAL_P(EG(user_error_handler)), 1);

			/* Create a new handler */
			zend_ptr_stack_push(&AWARE_G(user_error_handlers), tmp);

			zval_dtor(EG(user_error_handler));
			ZVAL_STRING(EG(user_error_handler), "__jam_error_handler_callback", 1);
		} else {
			zval_ptr_dtor(&AWARE_G(user_error_handler));
			FREE_ZVAL(AWARE_G(user_error_handler));
			AWARE_G(user_error_handler) = NULL;
		}
	}
}

PHP_FUNCTION(jam_restore_error_handler)
{
	if (AWARE_G(orig_restore_error_handler)) {
		AWARE_G(orig_restore_error_handler)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

		if (AWARE_G(user_error_handler)) {
			zval_ptr_dtor(&AWARE_G(user_error_handler));
			//FREE_ZVAL(AWARE_G(user_error_handler));
			AWARE_G(user_error_handler) = NULL;
		}

		/* Delete the top element from our stack */
		if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
			zval *tmp = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
			zval_dtor(tmp);
			FREE_ZVAL(tmp);
			tmp = NULL;
			
			if (zend_ptr_stack_num_elements(&AWARE_G(user_error_handlers)) > 0) {
				tmp = (zval *)zend_ptr_stack_pop(&AWARE_G(user_error_handlers));
				zend_ptr_stack_push(&AWARE_G(user_error_handlers), tmp);
				
				if (AWARE_G(user_error_handler)) {
					zval_ptr_dtor(&AWARE_G(user_error_handler));
					//FREE_ZVAL(AWARE_G(user_error_handler));
				}
				ALLOC_INIT_ZVAL(AWARE_G(user_error_handler));
				ZVAL_STRING(AWARE_G(user_error_handler), Z_STRVAL_P(tmp), 1);
			}
		}
	}
}

static void _add_assoc_zval_helper(zval *event, char *name, uint name_len TSRMLS_DC)
{	
	zval **ppzval;
	
	if (PG(auto_globals_jit)) {
		zend_is_auto_global(name, name_len TSRMLS_CC);
	}
	
	if (zend_hash_find(&EG(symbol_table), name, name_len, (void **) &ppzval) == SUCCESS) {
		/* Make sure that freeing jam_array doesn't destroy superglobals */
		Z_ADDREF_PP(ppzval);
		add_assoc_zval(event, name, *ppzval);
	}	
}

/* event must be initialized with MAKE_STD_ZVAL or similar and array_init before sending here */
void php_jam_capture_error_ex(zval *event, int type, const char *error_filename, const uint error_lineno, zend_bool free_event, const char *format, va_list args TSRMLS_DC)
{
	zval **ppzval;
	va_list args_cp;
	int len;
	char *buffer;
	char uuid_str[PHP_AWARE_UUID_LEN + 1];
	
	TSRMLS_FETCH();
	
	/* Generate unique identifier */
	if (!php_jam_generate_uuid(uuid_str)) {
		php_jam_original_error_cb(E_WARNING TSRMLS_CC, "Failed to generate uuid");
		return;
	}

	/* Capture superglobals */
	if (AWARE_G(log_get)) {
		_add_assoc_zval_helper(event, "_GET", sizeof("_GET") TSRMLS_CC);
	}
	
	if (AWARE_G(log_post)) {
		_add_assoc_zval_helper(event, "_POST", sizeof("_POST") TSRMLS_CC);
	}
	
	if (AWARE_G(log_cookie)) {
		_add_assoc_zval_helper(event, "_COOKIE", sizeof("_COOKIE") TSRMLS_CC);
	}
	
	if (AWARE_G(log_session)) {
		_add_assoc_zval_helper(event, "_SESSION", sizeof("_SESSION") TSRMLS_CC);
	}
	
	if (AWARE_G(log_server)) {
		_add_assoc_zval_helper(event, "_SERVER", sizeof("_SERVER") TSRMLS_CC);
	}
	
	if (AWARE_G(log_env)) {
		_add_assoc_zval_helper(event, "_ENV", sizeof("_ENV") TSRMLS_CC);
	}
	
	if (AWARE_G(log_files)) {
		_add_assoc_zval_helper(event, "_FILES", sizeof("_FILES") TSRMLS_CC);
	}
	
	/* Capture backtrace */
	if (AWARE_G(log_backtrace)) {
		zval *btrace;
		ALLOC_INIT_ZVAL(btrace);
#if PHP_API_VERSION <= PHP_5_3_X_API_NO
		zend_fetch_debug_backtrace(btrace, 0, 0 TSRMLS_CC);
#else
// TODO: introduce a directive for the amount of stack frames returned instead of hard coded 1000?
		zend_fetch_debug_backtrace(btrace, 0, 0 TSRMLS_CC,1000);
#endif
		add_assoc_zval(event, "backtrace", btrace);
	}
	
	va_copy(args_cp, args);
	len = vspprintf(&buffer, PG(log_errors_max_len), format, args_cp);
	va_end(args_cp);

	add_assoc_string(event,	"error_message", buffer, 0);
	add_assoc_string(event,	"filename",	(char *)error_filename, 1);
	
	add_assoc_long(event, "line_number", error_lineno);
	add_assoc_long(event, "error_type", type);
	
	/*
		Set the last logged uuid into _SERVER
	*/
	add_assoc_string(event, "jam_event_uuid", uuid_str, 1);
	add_assoc_long(event, "jam_event_time", time(NULL));

	/*
		Set the last logged uuid into _SERVER
	*/
	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &ppzval) == SUCCESS) {
		add_assoc_string(*ppzval, "jam_last_uuid", uuid_str, 1);
	}

	/* Send to backend */
	php_jam_storage_store_all(uuid_str, event, type, error_filename, error_lineno TSRMLS_CC);
	
	if (free_event) {
		zval_dtor(event);
		FREE_ZVAL(event);
	}
}

static void php_jam_user_event_trigger(int type TSRMLS_DC, const char *error_filename, const uint error_lineno, const char *format, ...)
{
	zval *event;
	va_list args;

	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	add_assoc_bool(event, "jam_event_trigger", 1);
	
	va_start(args, format);
	php_jam_capture_error_ex(event, type, error_filename, error_lineno, 1, format, args TSRMLS_CC);
	va_end(args);
}

void php_jam_invoke_handler(int type TSRMLS_DC, const char *error_filename, const uint error_lineno, const char *format, ...)
{
	zval *event;
	va_list args;

	ALLOC_INIT_ZVAL(event);
	array_init(event);
	
	va_start(args, format);
	php_jam_capture_error_ex(event, type, error_filename, error_lineno, 1, format, args TSRMLS_CC);
	va_end(args);
}

static void php_jam_display_error_page(char *filename) 
{
	php_stream *stream = php_stream_open_wrapper(filename, "r", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	
	if (stream) {
		char *buff;
		size_t buff_size;
		
		buff_size = php_stream_copy_to_mem(stream, &buff, PHP_STREAM_COPY_ALL, 0);
		php_stream_close(stream);
		
		if (buff_size) {
			PHPWRITE(buff, buff_size);
			efree(buff);
		}
	}
}

/* Wrapper that calls the original callback or our callback */
void php_jam_capture_error(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	TSRMLS_FETCH();
	
	/* Assume that display_errors if off */
	if (!PG(display_errors) && AWARE_G(error_page)) {
		if (type & E_PARSE || type & E_ERROR || type & E_COMPILE_ERROR || type & E_USER_ERROR) {
			php_jam_display_error_page(AWARE_G(error_page));
		}
	}
	
	if (type & AWARE_G(log_level)) {
		zval *event;

		ALLOC_INIT_ZVAL(event);
		array_init(event);

		php_jam_capture_error_ex(event, type, error_filename, error_lineno, 1, format, args TSRMLS_CC);
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);
	} else {
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);	
	}
}

/* Aware internal errors go through here */
MY_AWARE_EXPORTS void php_jam_original_error_cb(int type TSRMLS_DC, const char *format, ...)
{
	const char *error_filename;
	int error_lineno = 0;
	va_list args;

	error_filename = zend_get_executed_filename(TSRMLS_C);
	error_lineno   = zend_get_executed_lineno(TSRMLS_C);

	va_start(args, format);
	
	if (AWARE_G(orig_error_cb)) {
		AWARE_G(orig_error_cb)(type, error_filename, error_lineno, format, args);
	} else {
		zend_error_cb(type, error_filename, error_lineno, format, args);
	}
	
	va_end(args);
}

static PHP_INI_MH(OnUpdateLogLevel) 
{	
	if (!new_value) {
#if PHP_MAJOR_VERSION >= 5 && PHP_MINOR_VERSION >= 3
		AWARE_G(log_level) = E_ALL & ~E_NOTICE & ~E_STRICT & ~E_DEPRECATED;
#else 
		AWARE_G(log_level) = E_ALL & ~E_NOTICE & ~E_STRICT;
#endif
	} else {
		AWARE_G(log_level) = atoi(new_value);
	}
	return SUCCESS;
}

void php_jam_ini_parser_cb(zval *arg1, zval *arg2, zval *arg3, int callback_type, void *arg TSRMLS_DC)
{
	char *mod_name;
	long *level;
	
	if (!arg2) {
		return;
	}
	
	mod_name = Z_STRVAL_P(arg1);
	level    = malloc(sizeof(long));
	*level   = atol(Z_STRVAL_P(arg2));

	if (zend_hash_update(&AWARE_G(module_error_reporting), mod_name, strlen(mod_name) + 1, &level, sizeof(long), NULL) == FAILURE) {
		free(level);
	}
}

static PHP_INI_MH(OnUpdateModuleErrorReporting) 
{	
	int retval = SUCCESS;
	char *pch, *copy, *tok_buf;
	
	zend_hash_clean(&AWARE_G(module_error_reporting));
	
	if (!new_value || new_value_length == 0) {
		return retval;
	}
	
	/* Module error reporting is in format: mod_name=E_ALL,another_mod=E_WARNING */
	copy = estrdup(new_value);
	pch  = php_strtok_r(copy, ",", &tok_buf);
	
	while (pch != NULL) {
		char *buffer;
		int pch_len;
	
		pch_len = strlen(pch);

		/* Allocate a buffer */
		buffer = emalloc(pch_len + ZEND_MMAP_AHEAD);
		memset(buffer, 0, ZEND_MMAP_AHEAD);
		memcpy(buffer, pch, pch_len);
		
		if (zend_parse_ini_string(buffer, 0, ZEND_INI_SCANNER_NORMAL, php_jam_ini_parser_cb, NULL TSRMLS_CC) == FAILURE) {
			efree(buffer);
			retval = FAILURE;
			break;
		}
		
		efree(buffer);
		pch = php_strtok_r(NULL, ",", &tok_buf);
	}
	efree(copy);
	return retval;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam.enabled",			"1",		PHP_INI_PERDIR, OnUpdateBool, 		enabled,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.use_cache",        "0",	    PHP_INI_PERDIR, OnUpdateBool,	    use_cache,      	zend_jam_globals, jam_globals)
		
	STD_PHP_INI_ENTRY("jam.error_reporting",		"22519",	PHP_INI_PERDIR, OnUpdateLogLevel,	log_level,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.module_error_reporting", NULL,	PHP_INI_PERDIR, OnUpdateModuleErrorReporting,	module_error_reporting,			zend_jam_globals, jam_globals)
	
	
	STD_PHP_INI_ENTRY("jam.depth",			"10",		PHP_INI_PERDIR, OnUpdateLong,		depth,				zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_get",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_get,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_post",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_post,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_session",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_session,		zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_cookie",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_cookie,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_env",			"1",		PHP_INI_PERDIR, OnUpdateBool,		log_env,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_server",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_server,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_files",		"1",		PHP_INI_PERDIR, OnUpdateBool,		log_files,			zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.log_backtrace",	"1",		PHP_INI_PERDIR, OnUpdateBool,		log_backtrace,		zend_jam_globals, jam_globals)	
	STD_PHP_INI_ENTRY("jam.enable_event_trigger",	"1",		PHP_INI_PERDIR, OnUpdateBool,		enable_event_trigger,		zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.storage_modules",	NULL,		PHP_INI_PERDIR, OnUpdateString,		storage_modules,	zend_jam_globals, jam_globals)

	STD_PHP_INI_ENTRY("jam.slow_request_threshold",	"0",	PHP_INI_PERDIR, OnUpdateLong,	slow_request_threshold,	zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.memory_usage_threshold",	"0",	PHP_INI_PERDIR, OnUpdateLong,	memory_usage_threshold,	zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.source_baseurl",	"https://github.com/youruser/yourrepo",	PHP_INI_ALL, OnUpdateString,	source_baseurl,	zend_jam_globals, jam_globals)
	STD_PHP_INI_ENTRY("jam.appname",	"JaM",	PHP_INI_ALL, OnUpdateString,	appname,	zend_jam_globals, jam_globals)
	
	/* Display pretty error pages if display_errors=0 and the error is fatal */
	STD_PHP_INI_ENTRY("jam.error_page",	NULL,	PHP_INI_PERDIR, OnUpdateString,	error_page,	zend_jam_globals, jam_globals)
PHP_INI_END()

static int php_jam_long_dtor(void **datas TSRMLS_DC)
{
	long *st = *datas;
	
	if (st) {
		free(st);
	}
	return ZEND_HASH_APPLY_REMOVE;
}

PHP_GINIT_FUNCTION(jam)
{
	jam_globals->storage_modules	= NULL;
	jam_globals->enabled	   		= 1;

	jam_globals->log_level   		= 22519;

	jam_globals->depth	   		= 10;
	jam_globals->log_get	   		= 1;
	jam_globals->log_post	   		= 1;
	jam_globals->log_session 		= 1;
	jam_globals->log_cookie		= 1;
	jam_globals->log_server		= 1;
	jam_globals->log_files		= 1;
	jam_globals->log_env			= 1;
	jam_globals->log_backtrace	= 1;
	jam_globals->enable_event_trigger	= 1;
	
	jam_globals->use_cache        = 0;
	
	jam_globals->slow_request_threshold = 0;
	jam_globals->memory_usage_threshold = 0;
	
	jam_globals->orig_set_error_handler = NULL;
	jam_globals->user_error_handler     = NULL;
	
	jam_globals->source_baseurl = "https://github.com/youruser/yourrepo";
	jam_globals->appname = "JaM";
	jam_globals->error_page = NULL;

	php_jam_cache_init(&(jam_globals->s_cache));
	zend_hash_init(&(jam_globals->module_error_reporting), 0, NULL, (dtor_func_t)php_jam_long_dtor, 1);
} 

PHP_GSHUTDOWN_FUNCTION(jam)
{
	php_jam_cache_deinit(&(jam_globals->s_cache));	
	zend_hash_clean(&(jam_globals->module_error_reporting));
	zend_hash_destroy(&(jam_globals->module_error_reporting));
}

static void php_jam_override_error_handling(TSRMLS_D) 
{
	zend_function *orig_set_error_handler, *orig_restore_error_handler;
	
	AWARE_G(orig_error_cb) = zend_error_cb;
	zend_error_cb          =& php_jam_capture_error;
	
	if (zend_hash_find(EG(function_table), "set_error_handler", sizeof("set_error_handler"), (void **)&orig_set_error_handler) == SUCCESS) {
		AWARE_G(orig_set_error_handler) = orig_set_error_handler->internal_function.handler;
		orig_set_error_handler->internal_function.handler = zif_jam_set_error_handler;
	}
	if (zend_hash_find(EG(function_table), "restore_error_handler", sizeof("restore_error_handler"), (void **)&orig_restore_error_handler) == SUCCESS) {
		AWARE_G(orig_restore_error_handler) = orig_restore_error_handler->internal_function.handler;
		orig_restore_error_handler->internal_function.handler = zif_jam_restore_error_handler;
	}
	zend_ptr_stack_init(&AWARE_G(user_error_handlers));
}

PHP_RINIT_FUNCTION(jam)
{
	if (AWARE_G(enabled)) {
		php_jam_override_error_handling(TSRMLS_C);

#ifdef HAVE_GETTIMEOFDAY	
		if (AWARE_G(slow_request_threshold)) {
			if (!php_jam_init_slow_request_monitor(&AWARE_G(request_start_tv), &(AWARE_G(request_start_rusage)))) {
				AWARE_G(slow_request_threshold) = 0;
			}
		}
#endif
	}
	return SUCCESS;
}

static void php_jam_restore_error_handling(TSRMLS_D) 
{
	zend_function *orig_set_error_handler, *orig_restore_error_handler;
	
	zend_error_cb = AWARE_G(orig_error_cb);
	zend_ptr_stack_clean(&AWARE_G(user_error_handlers), ZVAL_DESTRUCTOR, 1);
	zend_ptr_stack_destroy(&AWARE_G(user_error_handlers));
	
	if (AWARE_G(user_error_handler)) {
		//zval_dtor(AWARE_G(user_error_handler));
		//zval_ptr_dtor(&AWARE_G(user_error_handler));
		//FREE_ZVAL(AWARE_G(user_error_handler));
	}
	
	if (zend_hash_find(EG(function_table), "set_error_handler", sizeof("set_error_handler"), (void **)&orig_set_error_handler) == SUCCESS) {
		orig_set_error_handler->internal_function.handler = AWARE_G(orig_set_error_handler);
	}
	if (zend_hash_find(EG(function_table), "restore_error_handler", sizeof("restore_error_handler"), (void **)&orig_restore_error_handler) == SUCCESS) {
		orig_restore_error_handler->internal_function.handler = AWARE_G(orig_restore_error_handler);
	}
}

PHP_RSHUTDOWN_FUNCTION(jam)
{
	if (AWARE_G(enabled)) {
#ifdef HAVE_GETTIMEOFDAY	
		if (AWARE_G(slow_request_threshold)) {
			php_jam_monitor_slow_request(&AWARE_G(request_start_tv), &AWARE_G(request_start_rusage), AWARE_G(slow_request_threshold));
		}
#endif
		if (AWARE_G(memory_usage_threshold)) {
			php_jam_monitor_memory_usage(AWARE_G(memory_usage_threshold) TSRMLS_CC);
		}
		/* restore error handler */
		php_jam_restore_error_handling(TSRMLS_C);
	}
	return SUCCESS;
}

static char *php_jam_mmap_error_page(const char *filename)
{
    char *ptr;
    struct stat st_buf;
    int fildes;
    size_t filesize;
    
    if (stat(filename, &st_buf) == -1) {
        return NULL;
    }

    fildes = open(filename, O_RDONLY);
    
    if (fildes == -1) {
        return NULL;
    }
    
    ptr = mmap(0, st_buf.st_size, PROT_READ, MAP_PRIVATE, fildes, 0);
    (void) close(fildes);
    
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return ptr;
}

/* {{{ PHP_MINIT_FUNCTION(jam) */
PHP_MINIT_FUNCTION(jam) 
{
	REGISTER_INI_ENTRIES();
	
	if (!AWARE_G(storage_modules)) {
		AWARE_G(enabled) = 0;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam) */
PHP_MSHUTDOWN_FUNCTION(jam)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam) */
PHP_MINFO_FUNCTION(jam)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam extension", "enabled");
	php_info_print_table_row(2, "jam version", PHP_AWARE_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_functions[] = {
	PHP_FE(jam_event_trigger, NULL)
	PHP_FE(jam_event_get, NULL)
	PHP_FE(jam_event_get_list, NULL)
	PHP_FE(jam_event_delete, NULL)
	PHP_FE(jam_storage_module_list, NULL)
	PHP_FE(__jam_error_handler_callback, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry jam_module_entry = {
	STANDARD_MODULE_HEADER,
	"jam",
	jam_functions,
	PHP_MINIT(jam),
	PHP_MSHUTDOWN(jam),
	PHP_RINIT(jam),
	PHP_RSHUTDOWN(jam),
	PHP_MINFO(jam),
	PHP_AWARE_EXTVER,
	
	PHP_MODULE_GLOBALS(jam),
	PHP_GINIT(jam),
	PHP_GSHUTDOWN(jam), /* GSHUTDOWN */
	NULL,
	
	STANDARD_MODULE_PROPERTIES_EX 
};

#ifdef COMPILE_DL_AWARE
ZEND_GET_MODULE(jam)
#endif
