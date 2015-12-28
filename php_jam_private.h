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

#ifndef _PHP_AWARE_PRIVATE_H_
# define _PHP_AWARE_PRIVATE_H_

/* for HAVE_GETTIMEOFDAY */
#ifdef PHP_WIN32
# include "win32/time.h"
#else
# include "main/php_config.h"
#endif

#include "php_jam.h"
#include "php_jam_storage.h"
#include "php_jam_request.h"
#include "php_jam_uuid.h"
#include "php_jam_cache.h"

/* {{{ void php_jam_capture_error_ex(zval *, int, const char *, const uint, zend_bool, const char *, va_list);
*/
void php_jam_capture_error_ex(zval *, int, const char *, const uint, zend_bool, const char *, va_list);
/* }}} */

/* {{{ void php_jam_invoke_handler(int TSRMLS_DC, const char *, const uint, const char *, ...);
*/
void php_jam_invoke_handler(int TSRMLS_DC, const char *, const uint, const char *, ...);
/* }}} */

#ifndef Z_ADDREF_PP
# define Z_ADDREF_PP(ppz) (*ppz)->refcount++;
#endif

#endif
