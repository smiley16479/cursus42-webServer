#!/bin/php
<?php
	print "<!doctype html>\n";
	print "<html>\n";
	print "\t<head>\n";
	print "\t\t<title>Variables</title>\n";
	print "\t</head>\n";
	print "\t<body>\n";
	print "\t\t<p>\n";
	if ($_GET)
	{
		print "\t\t\t";
		print $_GET["a"];
		print "\n";
		print "\t\t\t";
		print $_GET["b"];
		print "\n";
	}
	else
		print "Yo";
	print "\t\t</p>\n";
	print "\t</body>\n";
	print "</html>\n"
?>
