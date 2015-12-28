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

#include "php_jam_files.h"
#include "ext/standard/php_filestat.h"

ZEND_DECLARE_MODULE_GLOBALS(jam_files)

php_jam_storage_module php_jam_storage_module_files = {
	PHP_JAM_STORAGE_MOD(files)
};

PHP_JAM_CONNECT_FUNC(files)
{
	return AwareOperationNotSupported;
}

PHP_JAM_GET_FUNC(files)
{
	char filename[MAXPATHLEN], *buff;
	php_stream *stream;
	size_t buff_size;
	zend_bool status;

	if (snprintf(filename, MAXPATHLEN, "%s/%s.jam", JAM_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailed;
	}
	
	jam_printf("Filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "r", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	
	if (!stream) {
		return AwareOperationFailed;
	}
	
    buff_size = php_stream_copy_to_mem(stream, &buff, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream);

    if (!buff_size) {
		return AwareOperationFailed;
    }

	status = php_jam_storage_unserialize(buff, buff_size, event TSRMLS_CC);
	efree(buff);
	
	if (!status) {
		return AwareOperationFailed;
	}
	return AwareOperationSuccess;
}

PHP_JAM_STORE_FUNC(files)
{
	char filename[MAXPATHLEN];
	php_stream *stream;
	smart_str string = {0};

	if (snprintf(filename, MAXPATHLEN, "%s/%s.jam", JAM_FILES_G(storage_path), uuid) <= 0) {
		return AwareOperationFailed;
	}
	
	jam_printf("Storage filename: %s\n", filename);
	
	stream = php_stream_open_wrapper(filename, "w+", ENFORCE_SAFE_MODE & ~REPORT_ERRORS, NULL);
	
	if (!stream) {
		return AwareOperationFailed;
	}
	
	php_jam_storage_serialize(uuid, event, &string TSRMLS_CC);
	
	if (php_stream_write(stream, string.c, string.len) < string.len) {
		php_stream_close(stream);
		smart_str_free(&string);
		return AwareOperationFailed;
	}
	
	php_stream_close(stream);
	smart_str_free(&string);
	
	return AwareOperationSuccess;
}

/* Just the uuid part of the full path */
static int _php_jam_files_clean_path(zval **path TSRMLS_DC)
{
	if (Z_TYPE_PP(path) == IS_STRING && Z_STRLEN_PP(path) > PHP_JAM_UUID_LEN) {
		char *ptr, buffer[PHP_JAM_UUID_LEN + 1];
		memset(buffer, 0, PHP_JAM_UUID_LEN + 1);
		
		ptr = Z_STRVAL_PP(path) + (Z_STRLEN_PP(path) - 42);
		memcpy(buffer, ptr, PHP_JAM_UUID_LEN);
		
		buffer[PHP_JAM_UUID_LEN] = '\0';
		efree(Z_STRVAL_PP(path));
		ZVAL_STRING(*path, buffer, 1);
		
		return ZEND_HASH_APPLY_KEEP;
	}
	return ZEND_HASH_APPLY_REMOVE;
}	

static int php_jam_sort_mtime(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket *f, *s;
	zval *first, *second, *stat_f, *stat_s;
	int result;

	f = *((Bucket **) a);
	s = *((Bucket **) b);
	
	first = *((zval **) f->pData);
	second = *((zval **) s->pData);
	
	convert_to_string(first);
	convert_to_string(second);
	
	MAKE_STD_ZVAL(stat_f);
	MAKE_STD_ZVAL(stat_s);
	
	php_stat(Z_STRVAL_P(first), Z_STRLEN_P(first), FS_MTIME, stat_f TSRMLS_CC);
	php_stat(Z_STRVAL_P(second), Z_STRLEN_P(second), FS_MTIME, stat_s TSRMLS_CC);

	result = (Z_LVAL_P(stat_f) < Z_LVAL_P(stat_s));
	
	zval_dtor(stat_f);
	FREE_ZVAL(stat_f);
	
	zval_dtor(stat_s);
	FREE_ZVAL(stat_s);

	return result;
}

PHP_JAM_GET_LIST_FUNC(files)
{
	char path[MAXPATHLEN];
	zval *fname;
	zval *args[1];

	/* Use php glob */
	MAKE_STD_ZVAL(fname);
	ZVAL_STRING(fname, "glob", 1);

	MAKE_STD_ZVAL(args[0]);
	snprintf(path, MAXPATHLEN, "%s/*-*-*-*-*.jam", JAM_FILES_G(storage_path));
	
	ZVAL_STRING(args[0], path, 1);

	call_user_function(EG(function_table), NULL, fname, events, 1, args TSRMLS_CC);
	zval_dtor(fname);
	FREE_ZVAL(fname);
	
	zval_dtor(args[0]);
	FREE_ZVAL(args[0]);
	
	if (Z_TYPE_P(events) == IS_ARRAY) {
		
		if (zend_hash_sort(Z_ARRVAL_P(events), zend_qsort, php_jam_sort_mtime, 0 TSRMLS_CC) == FAILURE) {
			return AwareOperationFailed;
		}
		
		if (zend_hash_num_elements(Z_ARRVAL_P(events)) > limit) {
			
			zval *slice_args[3], *events_copy;
		
			MAKE_STD_ZVAL(fname);
			ZVAL_STRING(fname, "array_slice", 1);

			slice_args[0] = events;
		
			MAKE_STD_ZVAL(slice_args[1]);
			ZVAL_LONG(slice_args[1], start);
		
			MAKE_STD_ZVAL(slice_args[2]);
			ZVAL_LONG(slice_args[2], limit);
		
			MAKE_STD_ZVAL(events_copy);
			call_user_function(EG(function_table), NULL, fname, events_copy, 3, slice_args TSRMLS_CC);
		
			zval_dtor(fname);
			FREE_ZVAL(fname);

			zval_dtor(events);
			ZVAL_ZVAL(events, events_copy, 1, 1);

			zval_dtor(slice_args[1]);
			FREE_ZVAL(slice_args[1]);
		
			zval_dtor(slice_args[2]);
			FREE_ZVAL(slice_args[2]);
		}
		zend_hash_apply(Z_ARRVAL_P(events), (apply_func_t)_php_jam_files_clean_path TSRMLS_CC);
	}
	
	return AwareOperationSuccess;
}

PHP_JAM_DELETE_FUNC(files)
{
	AwareOperationStatus status;
	char path[MAXPATHLEN];
	int path_len;
	zval *stat;

	path_len = snprintf(path, MAXPATHLEN, "%s/%s.jam", JAM_FILES_G(storage_path), uuid);
	
	MAKE_STD_ZVAL(stat);
	php_stat(path, path_len, FS_IS_FILE, stat TSRMLS_CC);
	
	status = AwareOperationFailed;
	
	if (Z_BVAL_P(stat) && VCWD_UNLINK(path) == SUCCESS) {
		status = AwareOperationSuccess;
	}
	zval_dtor(stat);
	FREE_ZVAL(stat);
	return status;
}

PHP_JAM_DISCONNECT_FUNC(files)
{
	return AwareOperationNotSupported;
}

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("jam_files.storage_path", "/tmp",	PHP_INI_PERDIR, OnUpdateString, storage_path,	zend_jam_files_globals, jam_files_globals)
PHP_INI_END()

static void php_jam_files_init_globals(zend_jam_files_globals *jam_files_globals)
{
	jam_files_globals->storage_path = NULL;	
}

static zend_bool php_jam_files_startup_check(TSRMLS_D)
{
	zval *stat;
	zend_bool retval = 1;

	MAKE_STD_ZVAL(stat);
	php_stat(JAM_FILES_G(storage_path), strlen(JAM_FILES_G(storage_path)), FS_IS_DIR, stat TSRMLS_CC);

	if (Z_TYPE_P(stat) != IS_BOOL || !Z_BVAL_P(stat)) {
		php_jam_original_error_cb(E_CORE_WARNING TSRMLS_CC, "Could not enable jam_files. %s is not a directory", JAM_FILES_G(storage_path));
		retval = 0;
	}
	zval_dtor(stat);
	FREE_ZVAL(stat);
	
	return retval;
}

/* {{{ PHP_MINIT_FUNCTION(jam_files) */
PHP_MINIT_FUNCTION(jam_files) 
{
	AwareModuleRegisterStatus reg_status;
	ZEND_INIT_MODULE_GLOBALS(jam_files, php_jam_files_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	
	reg_status = PHP_JAM_STORAGE_REGISTER(files);

	switch (reg_status) 
	{
		case AwareModuleRegistered:
			if (!php_jam_files_startup_check(TSRMLS_C)) {
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

/* {{{ PHP_MSHUTDOWN_FUNCTION(jam_files) */
PHP_MSHUTDOWN_FUNCTION(jam_files)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MINFO_FUNCTION(jam_files) */
PHP_MINFO_FUNCTION(jam_files)
{	
	php_info_print_table_start();
	php_info_print_table_row(2, "jam_files storage", "enabled");
	php_info_print_table_row(2, "jam_files version", PHP_JAM_FILES_EXTVER);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES(); 
}

static zend_function_entry jam_files_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry jam_files_module_entry = {
        STANDARD_MODULE_HEADER,
        "jam_files",
        jam_files_functions,
        PHP_MINIT(jam_files),
        PHP_MSHUTDOWN(jam_files),
        NULL,
        NULL,
        PHP_MINFO(jam_files),
    	PHP_JAM_FILES_EXTVER,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_JAM_FILES
ZEND_GET_MODULE(jam_files)
#endif