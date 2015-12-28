#!/usr/bin/env php
<?php

// use this file to create the skeleton for additional storage backends.

define('SKELETON_BASE', "./skeleton");

if (!isset($argv[1])) {
    printf("Usage: %s <backend-ext-name>\n", $argv[0]);
    exit(1);
}

$module = $argv[1];

if (is_dir($module)) {
    printf("Module '$module' already exists\n");
    exit(2);
}

$module_ucase = strtoupper($module);

if (!mkdir($module)) {
	printf("Failed to module directory\n");
	exit(3);
}

$files = array(SKELETON_BASE . '/jam_skeleton.c' => "$module/jam_$module.c", 
			   SKELETON_BASE . '/php_jam_skeleton.h' => "$module/php_jam_$module.h", 
			   SKELETON_BASE . '/config.m4' => "$module/config.m4");

foreach ($files as $source => $target) {
	
	echo "Creating $target\n";
	
	$contents = file_get_contents($source);
	
	if (!$contents) {
		printf("Failed to read file $source\n");
		exit(4);
	}
	
	$modified = str_replace(array('skeleton', 'SKELETON'), array($module, $module_ucase), $contents);
	if (!file_put_contents($target, $modified)) {
		printf("Failed to write file $target\n");
		exit(5);
	}
}

echo "Created $module\n";
