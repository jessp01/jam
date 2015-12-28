#!/usr/bin/env php
<?php

define('SKELETON_BASE', "./skeleton");

if (!isset($_SERVER['argv'][1])) {
    printf("Usage: %s <name>\n", $_SERVER['argv'][0]);
    exit(1);
}

$module = $_SERVER['argv'][1];

if (is_dir($module)) {
    printf("Module '$module' already exists\n");
    exit(1);
}

$module_ucase = strtoupper($module);

if (!mkdir($module)) {
	printf("Failed to module directory\n");
	exit(1);
}

$files = array(SKELETON_BASE . '/jam_skeleton.c' => "$module/jam_$module.c", 
			   SKELETON_BASE . '/php_jam_skeleton.h' => "$module/php_jam_$module.h", 
			   SKELETON_BASE . '/config.m4' => "$module/config.m4");

foreach ($files as $source => $target) {
	
	echo "Creating $target\n";
	
	$contents = file_get_contents($source);
	
	if (!$contents) {
		printf("Failed to read file $source\n");
		exit(1);
	}
	
	$modified = str_replace(array('skeleton', 'SKELETON'), array($module, $module_ucase), $contents);
	if (!file_put_contents($target, $modified)) {
		printf("Failed to write file $target\n");
		exit(1);
	}
}

echo "Created $module\n";
