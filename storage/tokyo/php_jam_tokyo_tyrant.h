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

#ifndef _PHP_JAM_TOKYO_TYRANT_H_
# define _PHP_JAM_TOKYO_TYRANT_H_

#include <tcrdb.h>

TCRDB *php_jam_tyrant_init();

zend_bool php_jam_tyrant_open(TCRDB *tyrant, const char *hostname, long port);

zend_bool php_jam_tyrant_optimize(TCRDB *tyrant);

zend_bool php_jam_tyrant_put(TCRDB *tyrant, const char *uuid, const char *event, int event_len);

zend_bool php_jam_tyrant_get(TCRDB *tyrant, const char *uuid, zval *return_value TSRMLS_DC);

zend_bool php_jam_tyrant_get_list(TCRDB *tyrant, long start, long limit, zval *return_value);

zend_bool php_jam_tyrant_delete(TCRDB *tyrant, const char *uuid);

zend_bool php_jam_tyrant_close(TCRDB *tyrant);

void php_jam_tyrant_deinit(TCRDB *tyrant);

#endif