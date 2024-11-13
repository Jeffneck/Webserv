#!/bin/bash

# Nom du serveur
SERVER="./webserv"

# Fichier des URLs
URLS="stressUrls.txt"

# Fichier de log pour Valgrind
VALGRIND_LOG="valgrind.log"

# Lancer le serveur avec Valgrind en arrière-plan
valgrind --leak-check=full --track-origins=yes --log-file=$VALGRIND_LOG $SERVER & PID=$!

sleep 1
echo "Serveur lancé avec PID : $PID"

# Attendre que le serveur soit prêt (ajustez le délai si nécessaire)
sleep 3

# Lancer Siege avec 10 utilisateurs pendant 1 minute
siege -b -c 10 -t 1M -f $URLS

# Terminer le serveur
kill $PID

# Attendre que le serveur se termine
wait $PID

# Afficher les descripteurs ouverts après le test
echo "Descripteurs ouverts après le test :"
lsof -p $PID

# Afficher le contenu du fichier valgrind.log
# echo "Résultats de Valgrind :"
# cat $VALGRIND_LOG
