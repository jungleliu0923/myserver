<?php
/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file client.php
 * @author liujun05(com@baidu.com)
 * @date 2013/12/24 15:52:38
 * @brief 
 *  
 **/

$address = '10.48.24.12';
$port = 2000;

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

$result = socket_connect($socket, $address, $port);



$in = "here is 1";
socket_write($socket, $in, strlen($in));

$out = socket_read($socket, 8192);
echo $out ."\n";

$in = "here is 2";
socket_write($socket, $in, strlen($in));

socket_close($socket);

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
?>
