import os
import signal

# Obtenir le PID du processus courant
pid = os.getpid()
os.kill(pid, signal.SIGTERM)  # Envoie un signal SIGTERM pour tuer le processus Python
