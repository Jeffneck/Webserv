fichier vide OK
serveur sans root OK
serveur sans listen OK

upload off => err 405 OK
upload store => changement OK
upload store => inexistant err500 OK
upload > client max body size = err 413 ok

cgi off = err 405 OK
cgi extension pas ok = err 405 OK
lancer hello.fake avec l' extension cgi setup a .fake => OK
lancer hello.py avec l' extension cgi setup a .fake => telecharge hello.py (OK puisque .py est desormais considere comme un static file et en application/octet stream)





# Checker avec valgrind : 
valgrind --leak-check=full --track-fds=yes ./test_webserv > stdout 2>stderr

checker les fd : 
recup l' id du programme
    ps aux | grep test_webserv | grep -v grep
utiliser ensuite :
    lsof -p <PID>

# stress tester le serveur
lancer le script de stress (le modifier si besoin) 
    ./stress_test.sh
modifier les Url testees
    stressUrls.txt


CHECKS FONCTIONNELS 

Vérification basique avec curl : OK
curl http://127.0.0.1:8080 

Acces au second serveur sur le meme ip:port avec curl : OK
curl http://127.0.0.1:8080 -H "Host:anotherhost"

Tester avec différentes méthodes HTTP :
GET : OK
curl -X GET http://127.0.0.1:8080

POST : OK
curl -X POST 127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Type: application/x-www-form-urlencoded" -d "name=testname&email=testemail&message=testmessage"

PUT : (ERR 501 OK)
curl -X PUT http://127.0.0.1:8080 -d 'key=value'

DELETE :(HTTP 204 OK)
touch app/website/uploads/testfile && curl -X DELETE http://127.0.0.1:8080/uploads/testfile

DELETE INTERDIT (ERR 405 OK)
touch app/website/cgi-bin/testfile && curl -X DELETE http://127.0.0.1:8080/cgi-bin/testfile || rm app/website/cgi-bin/testfile

DELETE UN FICHIER INEXISTANT (ERR 404 OK)
curl -X DELETE http://127.0.0.1:8080/uploads/testfile

DELETE UN DOSSIER (ERR 400 OK)
curl -X DELETE http://127.0.0.1:8080/uploads/


Tester avec des en-têtes personnalisés : (Page servie normalement OK) 
curl -X GET http://127.0.0.1:8080 -H "Authorization: Bearer token"

Envoyer des données JSON dans un POST :(ERR 415 OK => unsupported media type)
curl -X POST http://127.0.0.1:8080 -H "Content-Type: application/json" -d '{"key":"value"}'

Télécharger un fichier via HTTP (fichier telecharge a la racine OK) :
curl -O http://127.0.0.1:8080/static/index.html



1. 400 Bad Request (400 - Mauvaise requête)
Situations possibles :

Requête malformée : Le client envoie une requête HTTP avec une syntaxe incorrecte, ce qui empêche le serveur de la comprendre. Par exemple, une ligne de requête incomplète ou des en-têtes mal formatés.

//Cas ou cela fonctionne sans erreur
curl http://127.0.0.1:8080 -H "Host:localhost"
//on utilise un second hote valide
curl http://127.0.0.1:8080 -H "Host:anotherhost"
//on utilise un hote invalide,le serveur est le 1er par defaut pour l'IP:PORT
curl http://127.0.0.1:8080 -H "Host:invalidhost"

//sans en-tete host (ERR 400 OK)
curl http://127.0.0.1:8080 -H "Host:"

//Methode HTTP inexistente
curl -X INEXISTENT http://127.0.0.1:8080/


En-têtes invalides : Les en-têtes de la requête contiennent des valeurs invalides ou incohérentes, comme un en-tête Content-Length négatif ou non numérique.

// cas ou cela fonctionne OK
curl -v -X POST http://127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Length: 47" -d "name=ntest&email=etest%40test.com&message=mtest"

//cas ou cela ne fonctionne pas car Content-Length est negatif (ERR 400 OK)
curl -v -X POST http://127.0.0.1:8080/cgi-bin/display.py -H "Content-Length: -10" -d "somedata"


3. 403 Forbidden (403 - Accès interdit)
Situations possibles :

Permissions de fichier insuffisantes : Les permissions du système de fichiers empêchent le serveur de lire le fichier demandé, même si le chemin est correct.
//avant interdiction (OK)
curl http://127.0.0.1:8080/forbiddenFolder/forbiddenFile.txt

//apres interdiction (ERR 403 OK)
chmod 000 ./app/website/forbiddenFolder/forbiddenFile.txt && curl -v http://127.0.0.1:8080/forbiddenFolder/forbiddenFile.txt && chmod 777 ./app/website/forbiddenFolder/forbiddenFile.txt

Interdiction de lister le contenu : Si l'autoindex est désactivé pour un répertoire et qu'il n'y a pas de fichier d'index (comme index.html)
//Ce Dir est autoindex off (ERR 403 OK)
curl http://127.0.0.1:8080/forbiddenFolder/

4. 404 Not Found (404 - Page non trouvée)
Situations possibles :

Ressource inexistante : Le client demande une page, un fichier ou une ressource qui n'existe pas sur le serveur.
curl http://127.0.0.1:8080/nonexistent-page.html

Le client essaye de delete une ressource inexistante (ERR 404 OK)
curl -X DELETE http://127.0.0.1:8080/uploads/inexistentfile

Chemin incorrect : L'URL fournie par le client est incorrecte ou contient des erreurs de frappe.
curl http://127.0.0.1:8080/wrongpath/

5. 405 Method Not Allowed (405 - Méthode non autorisée)
Situations possibles :

Méthode HTTP non supportée : Le client utilise une méthode HTTP (comme PUT, DELETE, PATCH) que le serveur ne reconnaît pas ou n'accepte pas pour la ressource demandée.


Restriction sur les méthodes : Le serveur est configuré pour n'accepter que certaines méthodes pour une ressource donnée. Par exemple, une page qui n'accepte que GET et HEAD, et le client envoie une requête POST.
curl -v -X GET http://127.0.0.1:8080/secretFolder/secretFile.txt

6. 408 Request Timeout (408 - Délai d'attente dépassé)
Situations possibles :

Inactivité du client : Le client met trop de temps à envoyer l'intégralité de sa requête, et le serveur ferme la connexion après un certain délai.

Délai dépassé lors du téléchargement : Lors du téléversement de fichiers, si le client met trop de temps à envoyer les données, le serveur peut renvoyer cette erreur.
Telecharger un fichier trop gros

411 - Length recquired
faire un Post sans content length
curl -v -X POST http://127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Length:" -d "name=ntest&email=etest%40test.com&message=mtest"

413 - Request too long

provoquer l' erreur (telecharger un fichier plus gros que client_max_body size du fichier de config)(Err 413 OK)


414 - URI too long (limite choisie len>100) (Err 414 OK)
curl http://127.0.0.1:8080/URItoolooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong


415 - Unsupported media type 
Fonctionne :(OK)
curl -X POST http://127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Length: 47" -H "Content-Type: application/x-www-form-urlencoded" -d "name=ntest&email=etest%40test.com&message=mtest"

provoque err 415 : (Err 415 OK)
curl -X POST http://127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Length: 47" -H "Content-Type: unknownType" -d "name=ntest&email=etest%40test.com&message=mtest"

7. 500 Internal Server Error (500 - Erreur interne du serveur)
Situations possibles :

Erreur dans le code serveur : le lancement du CGI ne fonctionne pas comme prevu (erreur de pipe/fork/fcntl)
=> provoquer l' erreur en decommantant return false dans la fonction CgiProcess::start()


Échec du CGI : Le script Python exécuté via execve plante, retourne une erreur ou produit une sortie non conforme, ce qui empêche le serveur de traiter la réponse correctement.
curl http://127.0.0.1:8080/cgi-bin/error.py

Problème de permissions : Le serveur n'a pas les permissions nécessaires pour exécuter le script CGI ou accéder à une ressource requise.
chmod 000 ./app/website/cgi-bin/hello.py && curl http://127.0.0.1:8080/cgi-bin/hello.py && chmod 777 ./app/website/cgi-bin/hello.py






8. 501 Not Implemented (501 - Fonctionnalité non implémentée)
Situations possibles :

Méthode non implémentée : Le client utilise une méthode HTTP que le serveur ne reconnaît pas du tout, comme PUT et le serveur n'a pas de traitement pour cette méthode.
curl -X PUT http://127.0.0.1:8080/
curl -X TRACE http://127.0.0.1:8080/

Fonctionnalité non supportée : Le client demande une fonctionnalité que le serveur ne supporte pas, comme une certaine version du protocole HTTP.

9. 502 Bad Gateway (502 - Mauvaise passerelle)
Situations possibles :

Le programme se termine avec un code d' erreur 
curl http://127.0.0.1:8080/cgi-bin/error.py

Le programme est kill par un signal
curl http://127.0.0.1:8080/cgi-bin/killMe.py

On envoie 



Échec de communication avec le CGI : Si votre serveur agit comme une passerelle vers le script CGI et que la communication échoue (par exemple, le script tourne indefiniment)



10. 503 Service Unavailable (503 - Service indisponible)
Situations possibles :

Maintenance du serveur : Le serveur est temporairement hors service pour maintenance.

Surcharge du serveur : Le serveur est trop occupé pour traiter la requête en raison d'un trop grand nombre de connexions simultanées.

Limitation de ressources : Les ressources système (comme la mémoire ou le CPU) sont insuffisantes pour traiter la requête.


Python n'est pas disponible ???????????????


11. 504 Gateway Timeout (504 - Délai d'attente de la passerelle)
Situations possibles :

Timeout avec le CGI : Le script Python appelé via execve met trop de temps à répondre, et le serveur atteint le délai maximum configuré pour attendre une réponse.
curl http://127.0.0.1:8080/cgi-bin/infinite.py











Autres situations potentielles :
413 Payload Too Large (413 - Entité de la requête trop grande)
Situations possibles :

Fichier trop volumineux : Le client tente de téléverser un fichier dont la taille dépasse la limite maximale autorisée par le serveur.
415 Unsupported Media Type (415 - Type de média non supporté)
Situations possibles :

Type de contenu non supporté : Le client envoie une requête avec un type de contenu que le serveur ne supporte pas, par exemple, un Content-Type inconnu lors du téléversement d'un fichier.
431 Request Header Fields Too Large (431 - Champs d'en-tête de requête trop grands)
Situations possibles :

En-têtes trop volumineux : Les en-têtes de la requête sont trop volumineux, peut-être à cause de cookies trop grands ou d'en-têtes personnalisés excessifs.
