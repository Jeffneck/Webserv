#!/bin/bash

# Nom du serveur
SERVER="./webserv"

# Fichier de log pour l'utilisation de la mémoire et du CPU
PS_LOG="ps.log"

# Lancer le serveur sans Valgrind en arrière-plan
$SERVER & PID=$!

sleep 1
echo "Serveur lancé avec PID : $PID"

# Attendre que le serveur soit prêt (ajustez le délai si nécessaire)
sleep 3

# Démarrer la surveillance de l'utilisation de la mémoire et du CPU
# Lancer en arrière-plan une boucle qui exécute 'ps' toutes les secondes
(
    while kill -0 $PID 2>/dev/null; do
        ps -p $PID -o %mem,cmd >> $PS_LOG
        sleep 5
    done
) &

PS_MONITOR_PID=$!

# Lancer Siege pendant 1 min
siege -b -t 1M "http://127.0.0.1:8080/static/empty.html"

# Attendre un peu pour s'assurer que la boucle 'ps' capture les dernières mesures
sleep 3

# Arrêter la surveillance de l'utilisation de la mémoire et du CPU
kill $PS_MONITOR_PID

# Terminer le serveur
kill $PID

# Attendre que le serveur se termine
wait $PID

# Afficher le contenu du fichier ps.log
# echo "Utilisation de la mémoire et du CPU pendant le test :"
cat $PS_LOG
echo "" > $PS_LOG