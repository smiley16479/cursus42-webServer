#!/bin/php
<!doctype html>
<html>
	<link href="layout.css" rel="stylesheet" type="text/css">
    <link rel="icon" 
      type="image/png" 
      href="/favicon.ico">
	<head>
		<title>Post Form</title>
	</head>
	<body>
		<p>
<?php
	if ($_POST["things"])
	{
		print "\t\t\t";
		print "things=";
		if (!empty($_POST["things"]))
			print $_POST["things"];
		print "\n";
	}
	else
		print "undefined variable things";
?>
		</p>
	</body>
</html>
