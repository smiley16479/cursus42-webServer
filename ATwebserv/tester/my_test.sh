#!/usr/bin/bash

INTERVAL=5

BLUE="\e[1;34m"
blue=$(tput setaf 4)
RESET=$(tput sgr0)

default_conf () {

	IP=127.0.0.1
	PORT=8080

	printf "\e[1;32mTEST : curl -X GET http://${IP}:${PORT} $RESET\n"
	curl -X GET http://$IP:$PORT   # Devrait redoner l'index par default (server par default)
	sleep $INTERVAL
	printf "\e[1;32mTEST : curl -X GET http://${IP}:${PORT}/error_pages $RESET\n"
	curl -X GET http://$IP:$PORT/error_pages   # Devrait redoner la page de directory
	sleep $INTERVAL
	printf "\e[1;32mTEST : curl -X GET http://${IP}:${PORT}/nimp $RESET\n"
	curl -X GET http://$IP:$PORT/nimp   # Devrait redoner les pages d'erreur
	sleep $INTERVAL
	# curl -X DELETE http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
	# sleep $INTERVAL
	# curl -X GET http://$IP:$PORT/nimp   # Devrait delete le fichier ou dossier
}

simple_conf () {

	IP=127.0.0.1
	PORT=8081

	printf "\e[1;32mTEST : curl -X GET http://${IP}:${PORT}/downloads/downloads/index \e[0m\n"
	curl -X GET http://$IP:$PORT/downloads/downloads/index   # redoner les pages d'erreur
	sleep $INTERVAL
	curl -X GET http://$IP:$PORT/downloads/downloads/index.html   # Devrait redoner l'index 
	sleep $INTERVAL
	# Pour l'instant on est ds une boucle infini avec :
	curl -X POST http://$IP:$PORT/ # Test POST http://localhost:8080/ with a size of 0
}

default_conf



# comment : 
# Pourquoi ca ca marche (Test GET http://localhost:8080/directory/nop) ds le tester et pas ca (Test GET Expected 404 on http://localhost:8080/directory/Yeah)