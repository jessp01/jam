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

#ifdef PHP_WIN32
#include <rpc.h>

/* {{{ zend_bool php_jam_generate_uuid(char *buf)
*/
zend_bool php_jam_generate_uuid(char *buf) 
{
	UUID uuid;
	unsigned char *uuid_str;

	if (UuidCreate(&uuid) != RPC_S_OK) {
		return 0;
    }

    if (UuidToString(&uuid, &uuid_str) != RPC_S_OK) {
		return 0;
    }

	if (strlen(uuid_str) > PHP_JAM_UUID_LEN) {
		RpcStringFree(&uuid_str);
		return 0;
	}

	strncpy(buf, uuid_str, PHP_JAM_UUID_LEN);
	buf[PHP_JAM_UUID_LEN] = '\0';
	
    RpcStringFree(&uuid_str);
	return 1;
}
/* }}} */

#else
#include <uuid/uuid.h>

/* {{{ zend_bool php_jam_generate_uuid(char *buf) 
*/
zend_bool php_jam_generate_uuid(char *buf) 
{
	uuid_t identifier;
	
	uuid_generate(identifier);
	uuid_unparse(identifier, buf);
	return 1;
}
/* }}} */

#endif
