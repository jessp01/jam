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

#ifndef _PHP_JAM_SPREAD_H_
# define _PHP_JAM_SPREAD_H_

#define PHP_JAM_SPREAD_EXTVER "0.0.1-dev"

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "../../php_jam.h"
#include "../../php_jam_storage.h"

#include <sp.h>

ZEND_BEGIN_MODULE_GLOBALS(jam_spread)
	
	zend_bool enabled;
	
	char *spread_name;
	char *group_name;
	char *user_name;
	
	mailbox spread_mailbox;
	
	zend_bool connected;
	
ZEND_END_MODULE_GLOBALS(jam_spread)

ZEND_EXTERN_MODULE_GLOBALS(jam_spread)

#ifdef ZTS
# define JAM_SPREAD_G(v) TSRMG(jam_spread_globals_id, zend_jam_spread_globals *, v)
#else
# define JAM_SPREAD_G(v) (jam_spread_globals.v)
#endif

/* Hook into jam module */
extern php_jam_storage_module php_jam_storage_module_spread;
#define php_jam_storage_module_spread_ptr &php_jam_storage_module_spread

/* Normal PHP entry */
extern zend_module_entry jam_spread_module_entry;
#define phpext_jam_spread_ptr &jam_spread_module_entry

PHP_JAM_STORAGE_FUNCS(spread);

#endif
