--TEST--
Test jam loading
--SKIPIF--
<?php if (!extension_loaded("jam")) print "skip"; ?>
--FILE--
<?php
echo "Aware loaded\n";
var_dump(ini_get("jam.enabled"));
?>
--EXPECT--
Aware loaded
string(1) "1"