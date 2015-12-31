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

#include "php_jam_stomp.h"

#include "ext/standard/php_smart_string.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_var.h"

php_jam_stomp_handle *php_jam_stomp_init() {
	php_jam_stomp_handle *handle;
	
	handle = malloc(sizeof(php_jam_stomp_handle));
	
	if (!handle)
		return NULL;

	handle->stream		= NULL;
	handle->connected   = 0;
	
	return handle;
}

zend_bool php_jam_stomp_send_frame(php_jam_stomp_handle *handle, 
	const char *command, smart_string *headers, smart_string *body, zend_bool expect_response, smart_string *response TSRMLS_DC) 
{
	zend_bool done = 0;
	zend_bool status;
	smart_string sendbuf = {0};
	char *buf, *line;
	size_t read_len;

	smart_str_appends(&sendbuf, command);
	smart_str_appendc(&sendbuf, '\n');
	
	if (headers && headers->len > 0) {
		smart_str_appendl(&sendbuf, headers->c, headers->len);
		smart_str_appendc(&sendbuf, '\n');
	}
	
	if (body && body->len > 0) {
		smart_str_appendl(&sendbuf, body->c, body->len);
		smart_str_appendc(&sendbuf, '\n');
	}
	
	smart_str_appendc(&sendbuf, '\n');
	smart_str_0(&sendbuf);

	status = (php_stream_write(handle->stream, sendbuf.c, sendbuf.len + 1) == sendbuf.len + 1);
	smart_str_free(&sendbuf);
	
	if (!status) {
		return 0;
	}
	
	if (expect_response && response) {
		while (!done) {
			/* I guess 1024 is just as good magic number as any other */
			buf  = safe_emalloc(1024 + 1, sizeof(char), 0);
			line = php_stream_get_line(handle->stream, ZSTR(buf), 1024, &read_len);
			
			/* The last line seems to contain just \n */
			if (!line || read_len == 1) 
				done = 1;
			
			smart_str_appends(response, line);
			efree(buf);
		}
		smart_str_0(response);
	}
	return 1;
}

zend_bool php_jam_stomp_connect(php_jam_stomp_handle *handle, 
	const char *server_uri, const char *username, const char *password, char **err_msg, int *err_code TSRMLS_DC) 
{
	struct timeval tv;
	smart_string headers  = {0};
	smart_string response = {0};
	zend_bool retval   = 0;

	handle->stream = php_stream_xport_create(server_uri, strlen(server_uri),
											ENFORCE_SAFE_MODE & ~REPORT_ERRORS,
											STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT,
											NULL, &tv, NULL, err_msg, err_code);

	if (!handle->stream) {
		return retval;
	}
	
	if (username && password) {
		smart_str_appends(&headers, "login:");
		smart_str_appends(&headers, username);
		smart_str_appendc(&headers, '\n');
		
		smart_str_appends(&headers, "passcode:");
		smart_str_appends(&headers, password);
		smart_str_appendc(&headers, '\n');
		smart_str_0(&headers);
		
		if (!php_jam_stomp_send_frame(handle, "CONNECT", &headers, NULL, 1, &response TSRMLS_CC)) {
			smart_str_free(&headers);
			php_stream_close(handle->stream);
			return retval;
		}
		smart_str_free(&headers);
	} else {
		if (!php_jam_stomp_send_frame(handle, "CONNECT", NULL, NULL, 1, &response TSRMLS_CC)) {
			php_stream_close(handle->stream);
			return retval;
		}
	}
	
	if (response.len) {
		if (response.len >= 9 && !strncmp(response.c, "CONNECTED", 9)) {
			handle->connected  = 1;
			retval = 1;
		} else {
			php_stream_close(handle->stream);
		}
		smart_str_free(&response);
	}	
	return retval;
}

zend_bool php_jam_stomp_send(php_jam_stomp_handle *handle, const char *queue_name, const char *message, long message_len TSRMLS_DC) 
{
	zend_bool retval;
	smart_string headers = {0};
	smart_string body    = {0};
	
	if (!handle->connected) {
		return 0;
	}

	/* Headers, must be terminated with \n */
	smart_str_appends(&headers, "destination:");
	smart_str_appends(&headers, queue_name);
	smart_str_appendc(&headers, '\n');
	
	/* Add content length header as well */
	smart_str_appends(&headers, "content-length:");
	smart_str_append_long(&headers, message_len);
	smart_str_appendc(&headers, '\n');
	smart_str_0(&headers);
	
	/* Body */
	smart_str_appendl(&body, message, message_len);
	
	retval = php_jam_stomp_send_frame(handle, "SEND", &headers, &body, 0, NULL TSRMLS_CC);
	smart_str_free(&headers);
	smart_str_free(&body);

	return retval;
}

void php_jam_stomp_disconnect(php_jam_stomp_handle *handle TSRMLS_DC) 
{
	if (!handle->connected)
		return;

	(void) php_jam_stomp_send_frame(handle, "DISCONNECT", NULL, NULL, 0, NULL TSRMLS_CC);
	php_stream_close(handle->stream);
	handle->connected = 0;
}

void php_jam_stomp_deinit(php_jam_stomp_handle *handle TSRMLS_DC) {

	if (handle->connected) {
		php_jam_stomp_disconnect(handle TSRMLS_CC);
	}
	free(handle);
}

