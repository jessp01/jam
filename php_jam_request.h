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

#ifndef _PHP_JAM_REQUEST_H_
# define _PHP_JAM_REQUEST_H_

#include "php_jam_private.h"

/* {{{ void php_jam_monitor_memory_usage(long threshold TSRMLS_DC);
*/
void php_jam_monitor_memory_usage(long threshold TSRMLS_DC);
/* }}} */

#ifdef HAVE_GETTIMEOFDAY
/* {{{ zend_bool php_jam_init_slow_request_monitor(struct timeval *, struct rusage *);
*/
zend_bool php_jam_init_slow_request_monitor(struct timeval *, struct rusage *);
/* }}} */

/* {{{ void php_jam_monitor_slow_request(struct timeval *, struct rusage *, long);
*/
void php_jam_monitor_slow_request(struct timeval *, struct rusage *, long);
/* }}} */
#endif /* HAVE_GETTIMEOFDAY */

#endif /* _PHP_JAM_REQUEST_H_ */
