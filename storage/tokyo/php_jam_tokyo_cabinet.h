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

#ifndef PHP_JAM_TOKYO_CABINET_H
# define PHP_JAM_TOKYO_CABINET_H

#include <tcutil.h>
#include <tctdb.h>

#include "php_jam_tokyo.h"

TCTDB *php_jam_cabinet_init();

zend_bool php_jam_cabinet_open(TCTDB *cabinet, const char *file_path, int mode);

zend_bool php_jam_cabinet_optimize(TCTDB *cabinet);

zend_bool php_jam_cabinet_put(TCTDB *cabinet, const char *uuid, const char *event, int event_len);

zend_bool php_jam_cabinet_get(TCTDB *cabinet, const char *uuid, zval *return_value TSRMLS_DC);

zend_bool php_jam_cabinet_get_list(TCTDB *cabinet, long start, long limit, zval *return_value);

zend_bool php_jam_cabinet_delete(TCTDB *cabinet, const char *uuid);

zend_bool php_jam_cabinet_close(TCTDB *cabinet);

void php_jam_cabinet_deinit(TCTDB *cabinet);

#endif