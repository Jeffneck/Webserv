#!/bin/bash

# Nom du serveur
SERVER="./test_webserv"

# Fichier des URLs
URLS="stressUrls.txt"

# Lancer le serveur avec Valgrind en arrière-plan
# valgrind --leak-check=full --track-fds=yes $SERVER & PID=$!

# lancer le serveur sans valgrind
$SERVER & PID=$!

echo "Serveur lancé avec PID : $PID"

# Attendre que le serveur soit prêt (ajustez le délai si nécessaire)
sleep 2

# Lancer Siege avec 5 users pendant 1min
siege -b -c 5 -t 1M -f $URLS

# Lancer Siege avec 100 users pendant 1min
# siege -b -c 50 -t 1M -f $URLS



# Terminer le serveur
kill $PID

# Attendre que le serveur se termine
wait $PID

# Afficher les descripteurs ouverts après le test
echo "Descripteurs ouverts après le test :"
lsof -p $PID

# Analyser les logs de Valgrind (si redirigé vers un fichier)
# cat valgrind.log
