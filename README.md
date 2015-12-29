# What is JaM?

JaM is a PHP monitoring system that supports storing PHP errors (events) into different storage backends. 

The events can later be retrieved from backends that implement PHP_JAM_GET_FUNC().

CREDIT line: JaM was initially forked from https://github.com/mkoppanen/php-aware which is no longer an active project. 

php-aware was developed by Mikko Koppanen.

# What does JaM monitor?

    - PHP errors of all levels [see http://php.net/manual/en/errorfunc.constants.php] 
    - Slow requests
    - Peak memory usage during request

# How does it work?

The jam extension overrides Zend's Engine zend_error_cb(), set_error_handler() and restore_error_handler() with a custom function that takes a copy of the current context, sends the error to the backends set in the jam.storage_modules directive and then calls the original error handler(s).

Each backend storage is a separate PHP extension and additional backends can therefore easily be added, see the "Creating additional storage backends" section. 

The backend will receive a zval * containing information about the current error which it then stores based on it's own configuration. 

It is possible to chain the backends to store the event in multiple backends, i.e: send an email and then log to Elasticsearch, for example.

The basic flow is:

  - JaM startup overrides zend_error_cb() with interceptor and stores a pointer to the original callback

  - PHP error happens: 
    * Zend Engine calls zend_error_cb() which passes the event to our callback
	* JaM main ext loops through all configured backends and passes the following:

```
const char *uuid; // uniq ID 
zval *event; // struct containing info about the event 
const char *error_filename; // filename in which the error occured 
long error_lineno; // line in which the error occured 
long type; // error type, see http://php.net/manual/en/errorfunc.constants.php 
const char *appname; // app identifier string, configured with the aware.appname directive but can be overridden from PHP code
```

	* the backend stores the event as defined in its PHP_JAM_STORE_FUNC()
    * call Zend Engine's original error callback 

# Basic setup    

## Debian/Ubuntu and friends
For Ubuntu 14.04 and any other deb based distro that uses php5 = 5.5.n, binary packages are available from Kaltura's CE repo:
```
# wget -O - http://installrepo.kaltura.org/repo/apt/debian/kaltura-deb.gpg.key|apt-key add -
# echo "deb [arch=amd64] http://installrepo.kaltura.org/repo/apt/debian kajam main" > /etc/apt/sources.list.d/kaltura.list
# aptitude install php5-jam php5-jam-elasticsearch php5-jam-email php5-jam-files
```
Then edit /etc/php5/mod-enabled/jam\*ini and set directives per your configuration.

To build debs for your deb based distribution, enter the debian dir under the root source dir and run:
```
$ dpkg-buildpackage -b -uc
```

## RHEL/CentOS and friends
The JaM extensions are available from Kaltura's CE repo.
To add the RHEL/CentOS 6 repo, add this to /etc/yum.repos.d/jam.list:
```
[Kaltura]
name = Kaltura Server
baseurl = http://installrepo.origin.kaltura.org/releases/latest/RPMS/$basearch/
gpgkey = http://installrepo.origin.kaltura.org/releases/RPM-GPG-KEY-kaltura
gpgcheck = 1
enabled = 1
```
To add the RHEL/CentOS 7 repo, add this to /etc/yum.repos.d/jam.list:
```
[Kaltura]
name = Kaltura Server
baseurl = http://installrepo.origin.kaltura.org/rhel7/latest/RPMS/$basearch/
gpgkey = http://installrepo.origin.kaltura.org/releases/RPM-GPG-KEY-kaltura
gpgcheck = 1
enabled = 1
```
To install the main JaM ext:
```
# yum install php-jam
```

To install the backends:
```
# yum install php-jam-email php-jam-elasticsearchi php-jam-snmp php-jam-files
```
Then set proper values in /etc/php.d/jam\*.ini and reload Apache if needed.

If you use a different distro that supports RPM, you can build the packages from php-jam.spec:
```
$ rpmbuild -bb --with email --with files --with snmp --with elasticsearch php-jam.spec
```


## Compiling from source

### Compiling the JaM extension
JaM depends on libuuid, make sure you install the relevant headers and shared objects for it.

On deb based systems:
```
# aptitude install uuid-dev
```
On RHEL based systems:
```
# yum install libuuid-devel
```

Other Linux and Unices distros should also have it in one format or another.

```
$ cd /path/to/jam/root/dir
$ phpize
$ ./configure
$ make
# make install
```

See "Core INI settings" for the available directives.

### Compiling backend extensions
By itself JaM will do pretty much nothing for you, next, select the backends you are interested in and cd into their dir under storage, for instance, if you are interested in the elasticsearch backend:
```
$ cd storage/elasticsearch
$ phpize
$ ./configure
$ make
# make install
```

See "elasticsearch->INI settings" for the relevant directives.

All available backends are under the storage dir, config and build instructions are the same for all.

## Core INI settings
 
<table>
	<tr>
		<td>Name</td>
		<td>Type</td>
		<td>Description</td>
		<td>Mode</td>
	</tr>
 	<tr>
		<td> jam.enabled </td>
		<td> boolean </td>
		<td> enable JaM (Default: On) </td>
		<td>PHP_INI_SYSTEM</td>
	</tr>
	<tr>
        <td> jam.use_cache </td>
 	    <td> boolean </td>
 	    <td> Use serialization cache (Default: On) </td>
	    <td>PHP_INI_PERDIR</td>
	</tr>
	<tr>		
        <td> jam.error_reporting </td>
		<td> integer </td>
		<td> Error reporting level (which events are stored) </td>
	    <td>PHP_INI_PERDIR</td>
	</tr>
	<tr>    
        <td> jam.module_error_reporting </td>
		<td> string </td>
		<td> Override error reporting on backend module basis (Format: elasticsearch=E_ALL,email=E_ERROR)</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.depth </td>
		<td> integer </td>
		<td> How many levels to serialize</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>	  
        <td> jam.log_get </td>
		<td> boolean </td>
		<td> Whether to include _GET values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_post </td>
		<td> boolean </td>
		<td> Whether to include _POST values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_session </td>
		<td> boolean </td>
		<td> Whether to include _SESSION values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_cookie </td>
		<td> boolean </td>
		<td> Whether to include _COOKIE values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_env </td>
		<td> boolean </td>
		<td> Whether to include _ENV values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_server </td>
		<td> boolean </td>
		<td> Whether to include _SERVER values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_files </td>
		<td> boolean </td>
		<td> Whether to include _FILES values in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.log_backtrace </td>
		<td> boolean </td>
		<td> Whether to include backtrace in the serialized event</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.enable_event_trigger </td>
		<td> boolean </td>
		<td> Whether to log events generated with jam_event_trigger</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
        <td> jam.storage_modules </td>
		<td> string </td>
		<td> Comma separated list of storage backend modules to enable (i.e aware.storage_modules="elasticsearch,email")</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>
        <td> jam.slow_request_threshold </td>
		<td> integer </td>
		<td> Setting > 0 activates slow request monitor (milliseconds)</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
	    <td> jam.memory_usage_threshold </td>
		<td> integer </td>
		<td> Setting > 0 activates memory usage monitor (bytes)</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
	    <td> jam.error_page </td>
		<td> string </td>
		<td> Error page filename. This page is displayed in case of a fatal error when display_errors is off</td>
	    <td>PHP_INI_PERDIR</td>
    </tr><tr>    
	    <td> jam.appname </td>
		<td> string </td>
		<td> report the appname in which the err was triggered </td>
	    <td>PHP_INI_ALL</td>
    </tr> 
</table>

# PHP functions:

* jam_event_trigger(int error_level, string message)

    Trigger an event. The event gets sent into configured storage backends but the internal error handler is not invoked.
    
    You can use this from your PHP code to send messages to the backend storage module.

* jam_event_get(string mod_name, string uuid)

    Get an event from storage backend module. Supported in 'files' and 'tokyo' backends.

* jam_event_get_list(string mod_name[, int start, int limit])
  
    Returns a list of events from the storage backend module. Supported in 'files' and 'tokyo' backends.
  
* jam_event_delete(string mod_name, string uuid)

    Deletes an event from storage backend module. Supported in 'files' and 'tokyo' backends.
    
* jam_storage_module_list()
  
    Returns a list of currently configured storage backend modules.



# Storage backends 

## elasticsearch
    Uses JSON-C and CURL libs to send an event to an ElasticSearch server
### Ini settings

<table>
	<tr>
		<td>Name</td>
		<td>Type</td>
		<td>Description</td>
		<td>Mode</td>
	</tr>
	<tr>
		<td> jam_elasticsearch.host </td>
		<td> String </td>
		<td> The ElasticSearch URL to send the event to</td>
		<td>PHP_INI_SYSTEM</td>
	</tr>
</table>

## email
    Sends an email containing information about the error
    
### Ini settings

<table>
	<tr>
		<td>Name</td>
		<td>Type</td>
		<td>Description</td>
		<td>Mode</td>
	</tr>
	<tr>
		<td> jam_email.to_address </td>
		<td> String </td>
		<td> Email recipient address. For example php_errors@example.com </td>
		<td>PHP_INI_PERDIR</td>
	</tr>
</table>

## snmp
    Sends the event as an SNMP trap
    
### Ini settings

<table>
	<tr>
		<td>Name</td>
		<td>Type</td>
		<td>Description</td>
		<td>Mode</td>
	</tr>
	<tr>
		<td> jam_snmp.trap_host </td>
		<td> String </td>
		<td> hostname:port of the snmptrapd </td>
		<td>PHP_INI_SYSTEM</td>
	</tr>
	<tr>
		<td> jam_snmp.trap_community </td>
		<td> String </td>
		<td> snmp community for the trap </td>
		<td>PHP_INI_PERDIR</td>
	</tr>
	<tr>
		<td> jam_snmp.trap_oid </td>
		<td> String </td>
		<td> OID for the trap </td>
		<td>PHP_INI_PERDIR</td>
	</tr>
	<tr>
		<td> jam_snmp.name_oid </td>
		<td> String </td>
		<td> OID for holding the script name </td>
		<td>PHP_INI_PERDIR</td>
	</tr>
	<tr>
		<td> jam_snmp.error_msg_oid </td>
		<td> String </td>
		<td> OID for holding the error message </td>
		<td>PHP_INI_PERDIR</td>
	</tr>		
	<tr>
		<td> jam_snmp.uuid_oid </td>
		<td> String </td>
		<td> OID for holding the uuid </td>
		<td>PHP_INI_PERDIR</td>
	</tr>		
</table>

## files
    Stores the information in files
    
### Ini settings

<table>
	<tr>
		<td>Name</td>
		<td>Type</td>
		<td>Description</td>
		<td>Mode</td>
	</tr>
	<tr>
		<td> jam_files.storage_path </td>
		<td> String </td>
		<td> Path to store the events to </td>
		<td>PHP_INI_PERDIR</td>
	</tr>
</table>

    
## spread
    Sends the event to a spread network

    This product uses software developed by Spread Concepts LLC for use in the Spread toolkit. 
    For more information about Spread see http://www.spread.org

    
### Ini settings

<table>
	<tr>
		<td> jam_spread.spread_name </td> 
		<td> String </td> 
		<td> port@hostname format of the spread daemon to connect to </td>
	</tr>
	<tr>
		<td> jam_spread.group_name </td> 
		<td> String </td> 
		<td> In which group to send the message to </td>
	</tr>		
	<tr>
		<td> jam_spread.user_name </td> 
		<td> String </td> 
		<td> Username of the sender, must be unique to the machine </td>
	</tr>
</table>

## stomp
    Sends the event to stomp message queue
    
### Ini settings

<table>
	<tr>
		<td> jam_stomp.server_uri </td> 
		<td> String </td> 
		<td> Uri of the server in tcp://hostname:port format </td>
	</tr>	
	<tr>
		<td> jam_stomp.queue_name </td> 
		<td> String </td> 
		<td> Name of the queue to send the message to  </td>
	</tr>
	<tr>
		<td> jam_stomp.username </td> 
		<td> String </td> 
		<td> Username to the queue (optional) </td>
	</tr>
	<tr>
		<td> jam_stomp.password </td> 
		<td> String </td> 
		<td> Password to the queue (optional) </td>
	</tr>		
</table>
    
## tokyo

    Stores into tokyo cabinet or tokyo tyrant
    
### Ini settings

<table>
	<tr>
		<td> jam_tokyo.backend </td>
		<td> String </td>
		<td> Can be 'cabinet' or 'tyrant'</td>
	</tr>
</table>

#### Tokyo Tyrant specific ini-settings


<table>
	<tr>
		<td> jam_tokyo.tyrant_host </td>
		<td> String </td>
		<td> Hostname if 'tyrant' is chosen as the backend </td>
	</tr>
	<tr>
		<td> jam_tokyo.tyrant_port </td>
		<td> Integer </td>
		<td> Port if 'tyrant' is chosen as the backend </td>
	</tr>
</table>


#### Tokyo Cabinet specific ini-settings

<table>
	<tr>
		<td> jam_tokyo.cabinet_file </td> 
		<td> String </td> 
		<td> Location of the cabinet file if cabinet is chosen </td>
	</tr>
	<tr>
		<td> jam_tokyo.cabinet_block </td> 
		<td> Boolean </td> 
		<td> Set to 'on' to use non-blocking locks </td>
	</tr>				
</table>

## zeromq2

    Sends events to zeromq2

### Ini settings

<table>
	<tr>
		<td> jam_zeromq2.dsn </td> 
		<td> String </td> 
		<td> Where to connect the publisher socket (Default: tcp://127.0.0.1:5555)</td>
	</tr>
	<tr>
		<td> jam_zeromq2.topic </td> 
		<td> String </td> 
		<td> Topic to publish the messages in (Default: jam)</td>
	</tr>		
</table>

    
    
## Creating additional storage backends
- run: 
``` 
./storage/create_backend_ext_skeleton.php <new-backend-ext-name>     
```

This will create a dir with all needed skeleton files for you.

- In, yourext/jam_yourext.c, implement the needed functions for your backend, minimum is to implement PHP_JAM_STORE_FUNC(yourext), other functions are not mandatory.
- If your extension has any directives, add them to PHP_INI_BEGIN() and init them in php_jam_yourext_init_globals() 
- If your extension depends on additional C/C++ libs, edit yourext/config.m4 accordingly.
- run the standard PHP configuration and build commands, i.e:
```
$ phpize
$ ./configure
$ make
# make install
```
