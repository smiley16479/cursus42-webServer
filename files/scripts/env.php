#!/bin/php
<!doctype html>
<html>
	<link href="./layout.css" rel="stylesheet" type="text/css">
    <link rel="icon" 
      type="image/png" 
      href="/favicon.ico">
	<head>
		<title>Env</title>
	</head>
	<body>
		<div class="header">
			<ul id="Menu">
				<li class="menuElem"><a class="menuButton" href="./layout.html">Homepage</a></li>
				<li class="menuElem"><a class="menuButton" href="./env.php">Environment Variables</a></li>
				<li class="menuElem"><a class="menuButton" href="./loop.php">Infinite loop</a></li>
				<li class="menuElem"><a class="menuButton" href="./phpinfo.php">About Php</a></li>
			</ul>
		</div>
		<div id ="Page">
			<p>
	<?php
		print "SERVER_SOFTWARE: ";
		print $_SERVER["SERVER_SOFTWARE"];
		print "<br>";
		print "SERVER_NAME: ";
		print $_SERVER["SERVER_NAME"];
		print "<br>";
		print "GATEWAY_INTERFACE: ";
		print $_SERVER["GATEWAY_INTERFACE"];
		print "<br>";
		print "SERVER_PROTOCOL: ";
		print $_SERVER["SERVER_PROTOCOL"];
		print "<br>";
		print "SERVER_PORT: ";
		print $_SERVER["SERVER_PORT"];
		print "<br>";
		print "REQUEST_METHOD: ";
		print $_SERVER["REQUEST_METHOD"];
		print "<br>";
		print "PATH_INFO: ";
		print $_SERVER["PATH_INFO"];
		print "<br>";
		print "PATH_TRANSLATED: ";
		print $_SERVER["PATH_TRANSLATED"];
		print "<br>";
		print "SCRIPT_FILENAME: ";
		print $_SERVER["SCRIPT_FILENAME"];
		print "<br>";
		print "SCRIPT_NAME: ";
		print $_SERVER["SCRIPT_NAME"];
		print "<br>";
		print "QUERY_STRING: ";
		print $_SERVER["QUERY_STRING"];
		print "<br>";
		print "REMOTE_HOST: ";
		print $_SERVER["REMOTE_HOST"];
		print "<br>";
		print "CONTENT_TYPE: ";
		print $_SERVER["CONTENT_TYPE"];
		print "<br>";
		print "CONTENT_LENGTH: ";
		print $_SERVER["CONTENT_LENGTH"];
		print "<br>";
		print "HTTP_USER_AGENT: ";
		print $_SERVER["HTTP_USER_AGENT"];
		print "<br>";
		print "HTTP_REFERER: ";
		print $_SERVER["HTTP_REFERER"];
		print "<br>";
	?>
			</p>
		</div>
		<div class="footer">
		</div>
	</body>
</html>
