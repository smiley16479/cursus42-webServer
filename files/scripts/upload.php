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
		</div>
		<div id="Page">
			<?php
				ob_end_clean();
				$uploaddir = "../../" . $_SERVER["TMPDIR"];
				$uploadfile = $uploaddir . basename($_FILES['cgi_upload']['name']);

				if (move_uploaded_file($_FILES['cgi_upload']['tmp_name'], $uploadfile))
					echo "Download Success !";
				else
					echo "Download Failure !";
			?>
			<br>
			<br>
			<?php
				echo 'Voici quelques informations de débogage :';
				print_r($_FILES);

			?>
			<br>
			<br>

			<?php
				echo "Uploaded File Preview :";
				$filetype = mime_content_type($uploadfile);
				echo "<br>";
				echo "<br>";
				$uploadfile = substr($uploadfile, strlen("../../"));

				if (substr($filetype, 0, strpos($filetype, "/"))  == "image")
				{
					echo "<img src=";
					echo substr($uploadfile, strpos($uploadfile, "/"));
					echo "/>";
				}
				else if (substr($filetype, 0, strpos($filetype, "/"))  == "video")
				{
					echo "<video>";
					echo "<source src=";
					echo substr($uploadfile, strpos($uploadfile, "/"));
					echo "/>";
					echo "</video>";
				}
				else
				{
					echo "No previsualisation is available for this MIME format";
					echo "<br>";
					echo "Your ressource has been added at this address :";
					echo "<a href=";
					echo substr($uploadfile, strpos($uploadfile, "/"));
					echo ">";
					echo basename($_FILES['cgi_upload']['name']);
					echo "</a>";
				}

				echo "<br>";

			?>

		</div>
		<div class="footer">
		</div>
	</body>
</html>
