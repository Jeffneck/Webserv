#!/bin/bash

# Nom du serveur
SERVER="./webserv configs/minimal.conf"


# Fichier de log pour Valgrind
VALGRIND_LOG="valgrind.log"

# Lancer le serveur avec Valgrind en arrière-plan
valgrind --leak-check=full --track-fds=yes --log-file=$VALGRIND_LOG $SERVER & PID=$!

sleep 1
echo "Serveur lancé avec PID : $PID"

# Attendre que le serveur soit prêt (ajustez le délai si nécessaire)
sleep 3

# Lancer Siege avec 50 utilisateurs pendant 1 minute
siege -b -c 50 -t1M "http://127.0.0.1:8080/static/empty.html"

# Terminer le serveur
kill $PID

# Attendre que le serveur se termine
wait $PID

# Afficher le contenu du fichier valgrind.log
echo "Résultats de Valgrind :"
cat $VALGRIND_LOG
