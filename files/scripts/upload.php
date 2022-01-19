
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
			$uploaddir = "../../" . $_SERVER["TMPDIR"];
			$uploadfile = $uploaddir . basename($_FILES['doodad']['name']);

			if (move_uploaded_file($_FILES['doodad']['tmp_name'], $uploadfile))
				echo "yyyyyyyyeaaaaaaaaaaaaaaaaah !";
			else
				echo "download failure !";
			echo "<br>";
			echo 'Voici quelques informations de débogage :';
			print_r($_FILES);


			?>
		</div>
		<div class="footer">
		</div>
	</body>
</html>
