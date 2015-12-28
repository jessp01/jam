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

#include "php_jam_tokyo.h"

TCTDB *php_jam_cabinet_init() 
{
	return tctdbnew();
}

zend_bool php_jam_cabinet_open(TCTDB *cabinet, const char *file_path, int mode)
{
	if (!tctdbopen(cabinet, file_path, mode)){
		return 0;
	}
	return 1;
}

zend_bool php_jam_cabinet_optimize(TCTDB *cabinet)
{
	return (tctdbsetindex(cabinet, "timestamp", TDBITDECIMAL) != 0);
}

zend_bool php_jam_cabinet_put(TCTDB *cabinet, const char *uuid, const char *event, int event_len) 
{	
	char timebuf[256];
	TCMAP *cols;
	int status;

	sprintf(timebuf, "%lu", (unsigned long)time(NULL));
	
	cols = tcmapnew();
	tcmapput2(cols, "timestamp",timebuf);
	tcmapput2(cols, "uuid", uuid);
	tcmapput(cols,  "event", strlen("event"), event, event_len);
	status = tctdbput(cabinet, uuid, strlen(uuid), cols);
	tcmapdel(cols);
	
	return (status != 0);
}

zend_bool php_jam_cabinet_get(TCTDB *cabinet, const char *uuid, zval *return_value TSRMLS_DC)
{
	zend_bool status = 0;
	TCMAP *cols;
	cols = tctdbget(cabinet, uuid, strlen(uuid));
  
	if (cols) {
		const char *event;
		int event_len;
		
		event = tcmapget(cols, "event", strlen("event"), &event_len);
		
		if (event) {
			status = php_jam_storage_unserialize(event, event_len, return_value TSRMLS_CC);
		}
		tcmapdel(cols);
    }
	return status;
}

zend_bool php_jam_cabinet_get_list(TCTDB *cabinet, long start, long limit, zval *return_value)
{
	TDBQRY *query;
	TCLIST *results;
	const char *pk;
	int pk_len, i;
	
	query = tctdbqrynew(cabinet);

	if (!query)
		return 0;
	
	tctdbqrysetorder(query, "timestamp", TDBQONUMDESC);
	tctdbqrysetlimit(query, limit, start);
	
	results = tctdbqrysearch(query);

	array_init(return_value);
	for (i = 0; i < tclistnum(results); i++){
		pk = tclistval(results, i, &pk_len);
		add_next_index_stringl(return_value, pk, pk_len, 1);
	}
	tclistdel(results);
  	tctdbqrydel(query);
	return 1;
}

zend_bool php_jam_cabinet_delete(TCTDB *cabinet, const char *uuid)
{
	return tctdbout2(cabinet, uuid);
}

zend_bool php_jam_cabinet_close(TCTDB *cabinet) 
{
	return (tctdbclose(cabinet) != 0);
}

void php_jam_cabinet_deinit(TCTDB *cabinet) 
{
	tctdbdel(cabinet);
}