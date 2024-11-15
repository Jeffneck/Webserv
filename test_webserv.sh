#!/bin/bash

# Script de test pour Webserv avec Siege et outils de surveillance
# Usage:
# ./test_webserv.sh [mode] [nombre_d_utilisateurs] [duree_en_minutes]
# Modes:
#   Aucun argument : Mode par défaut (Siege -b 1 user 1 minute, surveille avec htop et netstat)
#   1 : Surveiller avec ps
#   2 : Surveiller avec Valgrind
#   3 : Surveiller avec netstat
#   4 : Surveiller avec htop
# Arguments:
#   $2 : Nombre d'utilisateurs (Siege -c), défaut = 1
#   $3 : Durée du test en minutes (Siege -t), défaut = 1

# Fonction pour afficher l'utilisation correcte du script
usage() {
    echo "Usage: $0 [mode] [nombre_d_utilisateurs] [duree_en_minutes]"
    echo "Modes:"
    echo "  Aucun argument : Mode par défaut (Siege -b 1 user 1 minute)"
    echo "  1 : Surveiller avec ps"
    echo "  2 : Surveiller avec Valgrind"
    echo "  3 : Surveiller avec netstat"
    echo "  4 : Surveiller avec htop"
    echo "Arguments:"
    echo "  nombre_d_utilisateurs : Nombre de clients simultanés pour Siege (-c), défaut = 1"
    echo "  duree_en_minutes      : Durée du test en minutes (-t), défaut = 1"
    exit 1
}

# Vérifier les arguments
if [[ $# -gt 3 ]]; then
    usage
fi

# Assignation des arguments avec valeurs par défaut
MODE=${1:-0}
CONCURRENCY=${2:-1}
DURATION=${3:-1}

# Validation du mode
if ! [[ "$MODE" =~ ^[0-4]$ ]]; then
    echo "Erreur : Mode invalide. Doit être entre 0 et 4."
    usage
fi

# Validation des autres arguments
if ! [[ "$CONCURRENCY" =~ ^[0-9]+$ ]]; then
    echo "Erreur : Nombre d'utilisateurs doit être un entier positif."
    usage
fi

if ! [[ "$DURATION" =~ ^[0-9]+$ ]]; then
    echo "Erreur : Durée doit être un entier positif."
    usage
fi

# Variables
SERVER_CMD="./webserv"          # Commande pour démarrer votre serveur Webserv
SIEGE_URL="http://127.0.0.1:8080/static/empty.html"
VALGRIND_LOG="valgrind.log"
SERVER_PID=0
MONITOR_PID=0

# Fonction pour démarrer le serveur
start_server() {
    if [ "$MODE" -eq 2 ]; then
        echo "Démarrage du serveur avec Valgrind..."
        valgrind --leak-check=full --track-origins=yes --log-file=$VALGRIND_LOG $SERVER_CMD &
    else
        echo "Démarrage du serveur..."
        $SERVER_CMD &
    fi
    SERVER_PID=$!
    echo "Serveur démarré avec PID: $SERVER_PID"
    sleep 5  # Attendre que le serveur soit prêt
}

# Fonction pour arrêter le serveur
stop_server() {
    echo "Arrêt du serveur (PID: $SERVER_PID)..."
    kill -INT $SERVER_PID
    wait $SERVER_PID 2>/dev/null
    echo "Serveur arrêté."
}

# Fonction pour surveiller avec ps
monitor_ps() {
    echo "Surveillance avec ps (PID: $SERVER_PID)..."
    while kill -0 $SERVER_PID 2>/dev/null; do
        ps -p $SERVER_PID -o %mem,%cpu,cmd
        sleep 5
    done
}

# Fonction pour surveiller avec netstat
monitor_netstat() {
    echo "Surveillance des connexions avec netstat..."
    while kill -0 $SERVER_PID 2>/dev/null; do
        netstat -anp | grep 8080
        sleep 5
    done
}

# Fonction pour surveiller avec htop
monitor_htop() {
    echo "Lancement de htop pour surveiller le serveur (PID: $SERVER_PID)..."
    htop -p $SERVER_PID
}

# Fonction pour surveiller avec Valgrind
# Valgrind est déjà en cours d'exécution si MODE=2
monitor_valgrind() {
    echo "Valgrind est en cours d'exécution. Vérifiez le fichier $VALGRIND_LOG pour les détails."
    # Vous pouvez afficher en temps réel avec tail, mais cela peut devenir volumineux
    tail -f $VALGRIND_LOG &
    MONITOR_PID=$!
}

# Démarrer le serveur
start_server

# Exécuter la surveillance selon le mode
case $MODE in
    0)
        # Mode par défaut : Siege -b 1 user 1 minute, surveille avec htop et netstat
        echo "Mode par défaut : Siege -b $CONCURRENCY user(s) pour $DURATION minute(s)"
        # Lancer Siege en arrière-plan
        siege -b -c$CONCURRENCY -t${DURATION}M $SIEGE_URL &
        SIEGE_PID=$!
        # Lancer netstat en arrière-plan
        # monitor_netstat &
        # NETSTAT_PID=$!
        # Lancer htop (interactif, ne peut pas être en arrière-plan)
        # monitor_htop
        ;;
    1)
        # Mode 1 : Surveiller avec ps
        echo "Mode 1 : Surveiller avec ps"
        # Lancer Siege
        siege -b -c$CONCURRENCY -t${DURATION}M $SIEGE_URL &
        SIEGE_PID=$!
        # Lancer la surveillance avec ps en arrière-plan
        monitor_ps &
        MONITOR_PID=$!
        # Attendre la fin de Siege
        wait $SIEGE_PID
        ;;
    2)
        # Mode 2 : Surveiller avec Valgrind
        echo "Mode 2 : Surveiller avec Valgrind"
        # Lancer Siege
        siege -b -c$CONCURRENCY -t${DURATION}M $SIEGE_URL &
        SIEGE_PID=$!
        # Surveiller Valgrind
        monitor_valgrind
        # Attendre la fin de Siege
        wait $SIEGE_PID
        ;;
    3)
        # Mode 3 : Surveiller avec netstat
        echo "Mode 3 : Surveiller avec netstat"
        # Lancer Siege
        siege -b -c$CONCURRENCY -t${DURATION}M $SIEGE_URL &
        SIEGE_PID=$!
        # Lancer la surveillance avec netstat en arrière-plan
        monitor_netstat &
        NETSTAT_PID=$!
        # Attendre la fin de Siege
        wait $SIEGE_PID
        ;;
    4)
        # Mode 4 : Surveiller avec htop
        echo "Mode 4 : Surveiller avec htop"
        # Lancer Siege
        siege -b -c$CONCURRENCY -t${DURATION}M $SIEGE_URL &
        SIEGE_PID=$!
        # Lancer htop (interactif)
        monitor_htop
        ;;
    *)
        echo "Mode invalide. Veuillez utiliser un mode entre 0 et 4."
        stop_server
        exit 1
        ;;
esac

# Arrêter le serveur après les tests
stop_server

# Arrêter les processus de surveillance en arrière-plan
if [ "$MONITOR_PID" -ne 0 ]; then
    kill $MONITOR_PID 2>/dev/null
fi

if [ "$NETSTAT_PID" -ne 0 ]; then
    kill $NETSTAT_PID 2>/dev/null
fi

echo "Tests terminés."
