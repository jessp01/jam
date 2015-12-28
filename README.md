# What is jam?

jam is a PHP monitoring system that supports storing PHP errors (events) into different storage backends. 
The events can be later retrieved from backends that support retrieval.

# How does it work?

Roughly: override zend_error_cb, set_error_handler and restore_error_handler with a custom function that takes 
a copy of the current context, sends error to the configured backends and invokes original error handler(s).

The backend storage is abstracted from away and each backend storage is a separate PHP extension. 
The backend will receive a zval * containing information about the current error which it usually 
serializes and stores based on it's own configuration. 

Currently it is possible to chain the backends to allow configurations such as store the event to 
tokyo tyrant and send snmp trap as well.


So the cycle is a about the following:

  1. Aware startup
    -> override zend_error_cb with interceptor and store a pointer to the original callback

  2. PHP error happens 
    -> engine calls zend_error_cb which brings the error message etc to our callback
      -> loop through all configured backends and pass the information 
        -> the backend (normally) serializes the event and stores based on it's config
    -> call the original error callback 


# What does jam monitor?

  At the moment following things:
    - php errors 
    - slow requests
    - peak memory usage during request
    

# Available functions:

> jam_event_trigger(int error_level, string message)

    Trigger an event. The event gets sent into configured storage backends but the internal
    error handler is not invoked

> jam_event_get(string mod_name, string uuid)

    Get event from storage backend module. Supported in 'files' and 'tokyo' backends

> jam_event_get_list(string mod_name[, int start, int limit])
  
    Get list of events from storage backend module. Supported in 'files' and 'tokyo' backends
  
> jam_event_delete(string mod_name, string uuid)

    Delete event from storage backend module. Supported in 'files' and 'tokyo' backends
    
> jam_storage_module_list()
  
    Returns a list of currently configured storage backend modules


# GUI

There is a very simple GUI available in gui/ directory. It needs a lot of work.


# Spread backend

This product uses software developed by Spread Concepts LLC for use in the Spread toolkit. 
For more information about Spread see http://www.spread.org


# Storage backends 

## email
    Sends an email containing information about the error
    
### Ini settings

<table>
	<tr>
		<td> jam_email.to_address </td>
		<td> String </td>
		<td> Email recipient address. For example john.doe@example.com </td>
	</tr>
</table>

## files
    Stores the information in files
    
### Ini settings

<table>
	<tr>
		<td> jam_files.storage_path </td>
		<td> String </td>
		<td> Path to store the events to </td>
	</tr>
</table>

## snmp
    Sends an snmp trap
    
### Ini settings

<table>
	<tr>
		<td> jam_snmp.trap_host </td>
		<td> String </td>
		<td> hostname:port of the snmptrapd </td>
	</tr>
	<tr>
		<td> jam_snmp.trap_community </td>
		<td> String </td>
		<td> snmp community for the trap </td>
	</tr>
	<tr>
		<td> jam_snmp.trap_oid </td>
		<td> String </td>
		<td> OID for the trap </td>
	</tr>
	<tr>
		<td> jam_snmp.name_oid </td>
		<td> String </td>
		<td> OID for holding the script name </td>
	</tr>
	<tr>
		<td> jam_snmp.error_msg_oid </td>
		<td> String </td>
		<td> OID for holding the error message </td>
	</tr>		
	<tr>
		<td> jam_snmp.uuid_oid </td>
		<td> String </td>
		<td> OID for holding the uuid </td>
	</tr>		
</table>
    
## spread
    Sends the event to a spread network
    
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

# Ini settings

## Core settings
 
<table>
 	<tr>
        <td> jam.enabled </td>
		<td> boolean </td>
		<td> Is jam enabled (Default: On) </td>
    </tr><tr>
        <td> jam.use_cache </td>
 	    <td> boolean </td>
 	    <td> Use serialization cache (Default: On) </td>
    </tr><tr>		
        <td> jam.error_reporting </td>
		<td> integer </td>
		<td> Error reporting level (which events are stored) </td>
    </tr><tr>    
        <td> jam.module_error_reporting </td>
		<td> string </td>
		<td> Override error reporting on backend module basis (Format: tokyo=E_ALL,snmp=E_ERROR)</td>
    </tr><tr>    
        <td> jam.depth </td>
		<td> integer </td>
		<td> How many levels to serialize</td>
    </tr><tr>	  
        <td> jam.log_get </td>
		<td> boolean </td>
		<td> Whether to include _GET values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_post </td>
		<td> boolean </td>
		<td> Whether to include _POST values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_session </td>
		<td> boolean </td>
		<td> Whether to include _SESSION values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_cookie </td>
		<td> boolean </td>
		<td> Whether to include _COOKIE values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_env </td>
		<td> boolean </td>
		<td> Whether to include _ENV values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_server </td>
		<td> boolean </td>
		<td> Whether to include _SERVER values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_files </td>
		<td> boolean </td>
		<td> Whether to include _FILES values in the serialized event</td>
    </tr><tr>    
        <td> jam.log_backtrace </td>
		<td> boolean </td>
		<td> Whether to include backtrace in the serialized event</td>
    </tr><tr>    
        <td> jam.enable_event_trigger </td>
		<td> boolean </td>
		<td> Whether to log events generated with jam_event_trigger</td>
    </tr><tr>    
        <td> jam.storage_modules </td>
		<td> string </td>
		<td> List of storage backend modules to enabled (CSV)</td>
    </tr><tr>
        <td> jam.slow_request_threshold </td>
		<td> integer </td>
		<td> Setting > 0 activates slow request monitor (milliseconds)</td>
    </tr><tr>    
	    <td> jam.memory_usage_threshold </td>
		<td> integer </td>
		<td> Setting > 0 activates memory usage monitor (bytes)</td>
    </tr><tr>    
	    <td> jam.error_page </td>
		<td> string </td>
		<td> Error page filename. This page is displayed in case of a fatal error when display_errors is off</td>
    </tr> 
</table>
    
    
    
