
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
				echo "Coucou, l'image va s'afficher en dessous";
				$filetype = mime_content_type($uploadfile);
				echo "<br>";
				echo "Son type c'est : ";
				echo $filetype;
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
					echo basename($_FILES['doodad']['name']);
					echo "</a>";
				}

				echo "<br>";
				echo "Ici c'est la fin de la page";

			?>

		</div>
		<div class="footer">
		</div>
	</body>
</html>
