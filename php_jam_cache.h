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

#ifndef _PHP_AWARE_CACHE_H_
# define _PHP_AWARE_CACHE_H_

#include "php_jam_private.h"

/* {{{ void php_jam_cache_init(php_jam_serialize_cache *cache)
*/
void php_jam_cache_init(php_jam_serialize_cache *cache);
/* }}} */

/* {{{ void php_jam_cache_clean(php_jam_serialize_cache *cache)
*/
void php_jam_cache_clean(php_jam_serialize_cache *cache);
/* }}} */

/* {{{ zend_bool php_jam_cache_get(php_jam_serialize_cache *cache, const char *uuid, smart_str *retval)
*/
zend_bool php_jam_cache_get(php_jam_serialize_cache *cache, const char *uuid, smart_str *retval);
/* }}} */

/* {{{ void php_jam_cache_store(php_jam_serialize_cache *cache, const char *uuid, smart_str *data)
*/
void php_jam_cache_store(php_jam_serialize_cache *cache, const char *uuid, smart_str *data);
/* }}} */

/* {{{ void php_jam_cache_deinit(php_jam_serialize_cache *cache)
*/
void php_jam_cache_deinit(php_jam_serialize_cache *cache);
/* }}} */

#endif