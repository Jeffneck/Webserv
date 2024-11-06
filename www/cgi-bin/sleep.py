#!/usr/bin/env python3
import time

# En-tête CGI pour indiquer que la sortie est en HTML
print("Content-Type: text/html\n")

# Génération du contenu HTML
print("<html>")
print("<head><title>Pause de 10 secondes</title></head>")
print("<body>")
print("<h1>Le script commence, pause de 10 secondes...</h1>")

# Pause de 10 secondes
time.sleep(10)

print("<h1>Le script a terminé la pause de 10 secondes.</h1>")
print("</body>")
print("</html>")
