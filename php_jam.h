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

#ifndef _PHP_JAM_H_
# define _PHP_JAM_H_

#include "php.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_string.h"

#define PHP_JAM_EXTVER "0.1.0-dev"

#include <sys/resource.h>

#define PHP_5_3_X_API_NO                20090626
#define PHP_5_4_X_API_NO                20100525
#define PHP_5_5_X_API_NO                20121212
#define PHP_5_6_X_API_NO                20131226


/* Original error callback */
typedef void (*php_jam_orig_error_cb_t)(int, const char *, const uint, const char *, va_list);

typedef struct _php_jam_serialize_cache {
	char *data;
	
	int data_len;
	
	char *uuid;
	
	zend_bool has_item;
} php_jam_serialize_cache;

ZEND_BEGIN_MODULE_GLOBALS(jam)
	zend_bool enabled;			/* is the module enabled */
	zend_bool log_get;			/* whether to log get values */
	zend_bool log_post;			/* whether to log post values */
	zend_bool log_session;		/* whether to log session values */
	zend_bool log_cookie;		/* whether to log cookies */
	zend_bool log_env;			/* whether to log environment */
	zend_bool log_server;		/* whether to log server values */
	zend_bool log_files;		/* whether to log files array */
	
	zend_bool log_backtrace;	/* whether to store backtrace */

	zend_bool use_cache;		/* use serialize cache? stores repeated serializations */
	
	zend_bool enable_event_trigger;	/* do we want to log user generated events */

	long log_level;					/* what sort of events do we store */
	long depth;						/* how many levels to serialize (arrays) */
	long memory_usage_threshold;	/* how many bytes is the limit of excess usage */
	
	char *storage_modules;			/* which storage modules to enable */

	php_jam_orig_error_cb_t orig_error_cb;	/* the original zend_error_cb */

	void (*orig_set_error_handler)(INTERNAL_FUNCTION_PARAMETERS);	/* the set_error_handle entry */
	void (*orig_restore_error_handler)(INTERNAL_FUNCTION_PARAMETERS);	/* the restore error handler entry */
	
	zend_ptr_stack user_error_handlers;		/* previous user error handlers */
	zend_stack user_error_handler_levels;	/* the levels the user error handler handles */
	
	zval *user_error_handler;	/* the current active user error handler */
	
	long slow_request_threshold;	/* how many msec is considered slow, setting > 0 turns on the slow request monitor */
	struct timeval request_start_tv;	/* when the request started */
	struct rusage  request_start_rusage;	/* rsusage at the start of the request */

	HashTable module_error_reporting;	/* hashtable containing error reporting levels for different storage modules */
	
	php_jam_serialize_cache s_cache;	/* serialize cache, repeated serializations are stored here */
	char *source_baseurl; /* base URL of your code repo, for displaying a link to the file when reporting the error */	
	char *appname; /* report the appname in which the err was triggered */	
	char *error_page; /* Display pretty error page on fatal if set */
	
ZEND_END_MODULE_GLOBALS(jam)

ZEND_EXTERN_MODULE_GLOBALS(jam)

#ifdef ZTS
#  define JAM_G(v) TSRMG(jam_globals_id, zend_jam_globals *, v)
#else
#  define JAM_G(v) (jam_globals.v)
#endif

extern zend_module_entry jam_module_entry;
#define phpext_jam_ptr &jam_module_entry

#ifdef _JAM_DEBUG_
#  define jam_printf(...) fprintf (stderr, __VA_ARGS__)
#else
#  define jam_printf(...)
#endif

/*
	API exports
*/
#ifndef MY_JAM_EXPORTS
#  ifdef PHP_WIN32
#    define MY_JAM_EXPORTS __declspec(dllexport)
#  else
#    define MY_JAM_EXPORTS PHPAPI
#  endif
#endif

MY_JAM_EXPORTS void php_jam_original_error_cb(int type TSRMLS_DC, const char *format, ...);

#endif
