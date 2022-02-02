#!/bin/php

<!doctype html>
<html>
	<head>
		<link href="layout.css" rel="stylesheet" type="text/css">
		<link rel="icon" 
		  type="image/png" 
		  href="/favicon.ico">
		<title>Layout</title>
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
		<div id="Page">
			<div id="Content">
			<?php
				if (isset($_POST['send_up']))	{
					if (isset($_FILES['cgi_upload']) && $_FILES['cgi_upload']['error'] == UPLOAD_ERR_OK)	{

						$uploaddir = "../../" . $_SERVER["TMPDIR"];
						$uploadfile = $uploaddir . basename($_FILES['cgi_upload']['name']);

						echo	"<h1>";
						if (move_uploaded_file($_FILES['cgi_upload']['tmp_name'], $uploadfile))
							echo "Download Success !";
						else
							echo "Download Failure !";
						echo	"</h1>";

						echo	"<br>";
						echo	"tmp name :	";
						echo	$_FILES['cgi_upload']['tmp_name'];
						echo	"<br>";
						echo	"file name :	";
						echo	$_FILES['cgi_upload']['name'];
						echo	"<br>";
						echo	"file size :	";
						echo	$_FILES['cgi_upload']['size'];
						echo	"<br>";
						echo	"file type :	";
						echo	$_FILES['cgi_upload']['type'];
						echo	"<br>";
					}
				}
			?>
			<br>
			<br>

			<?php
				echo "Uploaded File Preview :";
				$filetype = mime_content_type($uploadfile);
				echo "<br>";
				echo "<br>";
				$uploadfile = substr($uploadfile, strlen("../../"));

				echo "<iframe src=";
				echo substr($uploadfile, strpos($uploadfile, "/"));
				echo "></iframe>";
				echo "<br>";

			?>

			</div>
		</div>
		<div class="footer">
		</div>
	</body>
</html>
