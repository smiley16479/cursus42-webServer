#!/bin/php-cgi
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
    <link rel="icon" 
      type="image/png" 
      href="/favicon.ico">
	<head>
		<title>get Form</title>
	</head>
	<body>
		<p>
<?php
	if ($_GET["stuff"])
	{
		print "\t\t\t";
		print "stuff=";
		print $_GET["stuff"];
		print "\n";
	}
	else
		print "undefined variable stuff";
?>
		</p>
	</body>
</html>
