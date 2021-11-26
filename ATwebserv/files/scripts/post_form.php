#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
	<head>
		<title>Post Form</title>
	</head>
	<body>
		<p>
<?php
	if ($_GET["things"])
	{
		print "\t\t\t";
		print "things=";
		print $_GET["things"];
		print "\n";
	}
	else
		print "undefined variable things";
?>
		</p>
	</body>
</html>
