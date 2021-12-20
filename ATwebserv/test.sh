#!/usr/bin/bash

INTERVAL=5

default_conf () {

	IP=127.0.0.1
	PORT=8080

	curl -X GET http://$IP:$PORT   # Devrait redoner l'index par default (server par default)
	sleep INTERVAL
	curl -X GET http://$IP:$PORT/error_pages   # Devrait redoner la page de directory
	sleep INTERVAL
	curl -X GET http://$IP:$PORT/nimp   # Devrait redoner les pages d'erreur
	sleep INTERVAL
	# curl -X DELETE http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
	# sleep INTERVAL
	# curl -X GET http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
}

simple_conf () {

	curl -X GET http://$IP:$PORT/downloads/downloads/index   # redoner les pages d'erreur
	sleep INTERVAL
	curl -X GET http://$IP:$PORT/downloads/downloads/index.html   # Devrait redoner l'index 
	sleep INTERVAL
}

default_conf