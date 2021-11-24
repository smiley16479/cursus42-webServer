#!/bin/php
<?php
	print "<!doctype html>\n";
	print "<html>\n";
	print "\t<head>\n";
	print "\t\t<title>Variables</title>\n";
	print "\t</head>\n";
	print "\t<body>\n";
	print "\t\t<p>\n";
	if ($_GET["stuff"])
	{
		print "\t\t\t";
		print "stuff=";
		print $_GET["stuff"];
		print "\n";
	}
	else
		print "undefined variable stuff";
	print "\t\t</p>\n";
	print "\t</body>\n";
	print "</html>\n"
?>
