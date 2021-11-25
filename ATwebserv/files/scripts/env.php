#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
	<head>
		<title>Env</title>
	</head>
	<body>
		<div id ="Page">
			<p>
	<?php
		print "SERVER_SOFTWARE: ";
		print $_GET["SERVER_SOFTWARE"];
		print "<br>";
		print "SERVER_NAME: ";
		print $_GET["SERVER_NAME"];
		print "<br>";
		print "GATEWAY_INTERFACE: ";
		print $_GET["GATEWAY_INTERFACE"];
		print "<br>";
		print "SERVER_PROTOCOL: ";
		print $_GET["SERVER_PROTOCOL"];
		print "<br>";
		print "SERVER_PORT: ";
		print $_GET["SERVER_PORT"];
		print "<br>";
		print "REQUEST_METHOD: ";
		print $_GET["REQUEST_METHOD"];
		print "<br>";
		print "PATH_INFO: ";
		print $_GET["PATH_INFO"];
		print "<br>";
		print "PATH_TRANSLATED: ";
		print $_GET["PATH_TRANSLATED"];
		print "<br>";
		print "SCRIPT_NAME: ";
		print $_GET["SCRIPT_NAME"];
		print "<br>";
		print "QUERY_STRING: ";
		print $_GET["QUERY_STRING"];
		print "<br>";
		print "REMOTE_HOST: ";
		print $_GET["REMOTE_HOST"];
		print "<br>";
		print "REMOTE_ADDR: ";
		print $_GET["REMOTE_ADDR"];
		print "<br>";
		print "AUTH_TYPE: ";
		print $_GET["AUTH_TYPE"];
		print "<br>";
		print "REMOTE_USER: ";
		print $_GET["REMOTE_USER"];
		print "<br>";
		print "REMOTE_IDENT: ";
		print $_GET["REMOTE_IDENT"];
		print "<br>";
		print "CONTENT_TYPE: ";
		print $_GET["CONTENT_TYPE"];
		print "<br>";
		print "CONTENT_LENGTH: ";
		print $_GET["CONTENT_LENGTH"];
		print "<br>";
		print "HTTP_ACCEPT: ";
		print $_GET["HTTP_ACCEPT"];
		print "<br>";
		print "HTTP_ACCEPT_LANGUAGE: ";
		print $_GET["HTTP_ACCEPT_LANGUAGE"];
		print "<br>";
		print "HTTP_USER_AGENT: ";
		print $_GET["HTTP_USER_AGENT"];
		print "<br>";
		print "HTTP_COOKIE: ";
		print $_GET["HTTP_COOKIE"];
		print "<br>";
		print "HTTP_REFERER: ";
		print $_GET["HTTP_REFERER"];
		print "<br>";
	?>
			</p>
		</div>
	</body>
</html>
