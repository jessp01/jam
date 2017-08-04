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

/* gettimeofday */
#ifdef PHP_WIN32
# include "win32/php_jam_win32.h"
#endif

#ifdef HAVE_GETTIMEOFDAY

# ifndef timersub

#define timersub(tvp, uvp, vvp) \
	do { \
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
		if ((vvp)->tv_usec < 0) { \
			(vvp)->tv_sec--; \
			(vvp)->tv_usec += 1000000; \
		} \
	} while (0);
	
# endif /* ifndef timersub */

#define timeval_to_msec(_my_tv) ((_my_tv.tv_sec * 1000) + (_my_tv.tv_usec / 1000))
	
#define timeval_to_double(_my_tv) (double)(_my_tv).tv_sec + ((double)(_my_tv).tv_usec / 1000000.0)

/* {{{ static void php_jam_capture_slow_request(long elapsed, long threshold, double u_time, double s_time, const char *format, ...)
*/
static void php_jam_capture_slow_request(long elapsed, long threshold, double u_time, double s_time, const char *format, ...)
{
	//TODO: REIMPLEMENT!
	/*va_list args;
	zval *slow_request;
	
	//array_init(event);
	
	array_init(slow_request);
	
	add_assoc_long(slow_request, "time_elapsed", elapsed);
	add_assoc_long(slow_request, "slow_request_threshold", threshold);	
	
	// Info about where the time is spent, TODO: check WIN32 
	add_assoc_double(slow_request, "rusage_user_time", u_time);
	add_assoc_double(slow_request, "rusage_system_time", s_time);
	
	add_assoc_bool(event, "slow_request", 1);
	add_assoc_zval(event, "_JAM_REQUEST", slow_request);

	va_start(args, format);
	php_jam_capture_error_ex(E_CORE_WARNING, "jam internal report", 0, 1, format, args);
	va_end(args);*/
}
/* }}} */

/* {{{ zend_bool php_jam_init_slow_request_monitor(struct timeval *request_start, struct rusage *request_start_rusage)
*/
zend_bool php_jam_init_slow_request_monitor(struct timeval *request_start, struct rusage *request_start_rusage)
{
	if (gettimeofday(request_start, NULL) == 0) {
		if (getrusage(RUSAGE_SELF, request_start_rusage) == 0) {
			return 1;
		}
	}
	return 0;
}
/* }}} */

/* {{{ void php_jam_monitor_slow_request(struct timeval *request_start, struct rusage *request_start_rusage, long threshold)
*/
void php_jam_monitor_slow_request(struct timeval *request_start, struct rusage *request_start_rusage, long threshold) 
{
	struct timeval request_end;
	
	if (gettimeofday(&request_end, NULL) == 0) {
		struct timeval request_diff;
		long elapsed;

		timersub(&request_end, request_start, &request_diff);
		elapsed = timeval_to_msec(request_diff); 

		if (elapsed > threshold) {
			double u_time = 0.0, s_time = 0.0;
			struct timeval tv_utime, tv_stime;
			struct rusage request_end_rusage;
			
			if (getrusage(RUSAGE_SELF, &request_end_rusage) == 0) {
				timersub(&request_end_rusage.ru_utime, &(request_start_rusage->ru_utime), &tv_utime);
				timersub(&request_end_rusage.ru_stime, &(request_start_rusage->ru_stime), &tv_stime);

				u_time = timeval_to_double(tv_utime); 
				s_time = timeval_to_double(tv_stime); 
			}
			php_jam_capture_slow_request(elapsed, threshold, u_time, s_time, "Slow request detected");
		}
	}
}
/* }}} */
#endif /* ifdef HAVE_GETTIMEOFDAY */

/* {{{ static void php_jam_capture_memory_usage(long peak, long threshold, const char *format, ...)
*/
static void php_jam_capture_memory_usage(long peak, long threshold, const char *format, ...)
{
	// TODO: reimplement
	/*va_list args;
	//zval *event, *peak_usage;
	
	//ALLOC_INIT_ZVAL(event);
	//array_init(event);
	
	//ALLOC_INIT_ZVAL(peak_usage);
	//array_init(peak_usage);

	add_assoc_long(peak_usage, "memory_peak_usage", peak);
	add_assoc_long(peak_usage, "memory_usage_threshold", threshold);	
	
	add_assoc_zval(event, "_JAM_MEMORY", peak_usage);
	add_assoc_bool(event, "excessive_memory_usage", 1);

	va_start(args, format);
	php_jam_capture_error_ex(E_CORE_WARNING, "jam internal report", 0, 1, format, args);
	va_end(args);*/
}
/* }}} */

/* {{{ void php_jam_monitor_memory_usage(long threshold TSRMLS_DC)
*/
void php_jam_monitor_memory_usage(long threshold TSRMLS_DC)
{
	long peak = zend_memory_peak_usage(1 TSRMLS_CC);

	if (peak > threshold) {
		php_jam_capture_memory_usage(peak, threshold, "Excessive memory usage detected");
	}
}
/* }}} */
