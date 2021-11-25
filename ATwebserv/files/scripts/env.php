#!/bin/php
<!doctype html>
<html>
	<head>
		<title>Env</title>
	</head>
	<body>
		<p>
<?php
	print $_Get["SERVER_SOFTWARE"];
	print $_Get["SERVER_NAME"];
	print $_Get["GATEWAY_INTERFACE"];
	print $_Get["SERVER_PROTOCOL"];
	print $_Get["SERVER_PORT"];
	print $_Get["REQUEST_METHOD"];
	print $_Get["PATH_INFO"];
	print $_Get["PATH_TRANSLATED"];
	print $_Get["SCRIPT_NAME"];
	print $_Get["QUERY_STRING"];
	print $_Get["REMOTE_HOST"];
	print $_Get["REMOTE_ADDR"];
?>
		</p>
	</body>
</html>
