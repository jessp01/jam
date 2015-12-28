--TEST--
Test set_error_handler overloading error levels
--SKIPIF--
<?php if (!extension_loaded("jam")) print "skip"; ?>
--FILE--
<?php

var_dump(ini_get("jam.enabled"));

function my_error_handler() {
    echo "Called\n";
}

set_error_handler('my_error_handler', E_WARNING);

this_constant_does_not_exist;

foreach (null as $k) {}

?>
--EXPECTF--
string(1) "1"

Notice: Use of undefined constant this_constant_does_not_exist - assumed 'this_constant_does_not_exist' in %s on line %d
Called