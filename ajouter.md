Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
=> prendre en compte cette directive dans les requetes chrome


upload des fichiers


gestion des cgi avec cgi on

code de retour avec return ? ou toujours redir permanente 301 (hardcoder)

verifier si aucune root n' est mise sur le serveur si une exception se lance


apprendre a utiliser siege avec plusieurs utilisateurs pour tester la fiabilite de mon serveur :
siege -c 100 -t 10S 127.0.0.1:8080
siege localhost:8080 

Mon webserv n' est pas non bloquant : 
boucles while sur des read ?
boucles while sur les write ?
waitpid sur le script python...


http://127.0.0.1:8080/cgi-bin/request.py?name=hugo
ne fonctionne pas car cela n' est pas considere comme un script python 


cgi quand pas cgi on
erreur detectee dans cgi = retour specifique du cgi = throw err
http://127.0.0.1:8080//cgi-bin/display.py = le fichier est telecharge plutot qu' exec quand double slash'
cgi qui tourne a l' infini = erreur ?



trop grosse requete > client max body size
trop grosse requete televersement

fichier de config vide

listen pas dans config

verifier les sockets ouverts en fin de process

