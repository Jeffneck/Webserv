#!/usr/bin/env python3

# print("Content-Type: text/html")
# print()
# print("<html>")
# print("<head><title>CGI Test</title></head>")
# print("<body>")
# print("<h1>Bonjour depuis un script CGI en Python!</h1>")
# print("</body>")
# print("</html>")

# import sys
# import time

# # Votre code qui génère la réponse CGI
# # print("Content-Type: text/html\n")
# print("<html><body><h1>Hello, World!</h1></body></html>")

# # Vider le buffer stdout pour s'assurer que toutes les données sont écrites
# sys.stdout.flush()
# time.sleep(3)
# sys.stdout.flush()

# # Fermer explicitement stdout
# sys.stdout.close()

from ptyprocess import PtyProcessUnicode
p = PtyProcessUnicode.spawn(['./test'])
p.write("123\n")
print(p.read())
p.sendeof()
print(p.read())