--TEST--
Test restore_error_handler
--SKIPIF--
<?php if (!extension_loaded("jam")) print "skip"; ?>
--FILE--
<?php

function my_error_handler() {
    echo "Called 1\n";
}

function my_second_handler() {
    echo "Called 2\n";
}

set_error_handler('my_error_handler');
restore_error_handler();

this_constant_does_not_exist;

var_dump(set_error_handler('my_error_handler'));
var_dump(set_error_handler('my_second_handler'));
restore_error_handler();

this_constant_does_not_exist;

restore_error_handler();

var_dump(set_error_handler('my_error_handler'));
var_dump(set_error_handler('my_second_handler'));
?>
--EXPECTF--
Notice: Use of undefined constant this_constant_does_not_exist - assumed 'this_constant_does_not_exist' in %s on line %d
NULL
string(16) "my_error_handler"
Called 1
NULL
string(16) "my_error_handler"