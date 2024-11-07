Checker avec valgrind : 
valgrind --leak-check=full ./test_webserv > stdout 2>stderr


CHECKS FONCTIONNELS 

Vérification basique avec curl :
curl http://127.0.0.1:8080
Tester avec différentes méthodes HTTP :
GET :
curl -X GET http://127.0.0.1:8080
POST :
curl -X POST http://127.0.0.1:8080 -d 'key=value'
PUT :
curl -X PUT http://127.0.0.1:8080 -d 'key=value'
DELETE :
curl -X DELETE http://127.0.0.1:8080
Tester avec des en-têtes personnalisés :
curl -X GET http://127.0.0.1:8080 -H "Authorization: Bearer token"
Envoyer des données JSON dans un POST :
curl -X POST http://127.0.0.1:8080 -H "Content-Type: application/json" -d '{"key":"value"}'
Tester la réponse avec verbose :
curl -v http://127.0.0.1:8080
Tester le délai d'attente (timeout) :
curl --max-time 5 http://127.0.0.1:8080
Vérifier si le serveur redirige (301 ou 302) :
curl -I http://127.0.0.1:8080
Télécharger un fichier via HTTP :
curl -O http://127.0.0.1:8080/fichier.txt
Tester une requête HTTPS (si applicable) :
curl -k https://127.0.0.1:8080
Tester avec telnet pour un test basique de connexion :
telnet 127.0.0.1 8080







1. 400 Bad Request (400 - Mauvaise requête)
Situations possibles :

Requête malformée : Le client envoie une requête HTTP avec une syntaxe incorrecte, ce qui empêche le serveur de la comprendre. Par exemple, une ligne de requête incomplète ou des en-têtes mal formatés.

echo -e "GET / HTTP/1.1\nHost: 127.0.0.1:8080\n\n" | nc 127.0.0.1 8080
//entete host vide OK
curl -v http://127.0.0.1:8080 -H "Host:"
//sans entete host OK
curl -v --http1.1 http://127.0.0.1:8080 -H "Host"


En-têtes invalides : Les en-têtes de la requête contiennent des valeurs invalides ou incohérentes, comme un en-tête Content-Length négatif ou non numérique.

// cas ou cela fonctionne OK
curl -v -X POST http://127.0.0.1:8080/cgi-bin/contactForm.py -H "Content-Length: 47" -d "name=ntest&email=etest%40test.com&message=mtest"

//cas ou cela ne fonctionne pas car Content-Length est negatif
curl -v -X POST http://127.0.0.1:8080/cgi-bin/display.py -H "Content-Length: -10" -d "name=ntest&email=etest%40test.com&message=mtest"

Encodage incorrect : Le corps de la requête est encodé dans un format que le serveur ne peut pas décoder, ou l'en-tête Content-Encoding spécifie un encodage non supporté.
curl -v http://127.0.0.1:8080 -H "Content-Encoding: unsupported-encoding"

Requête trop longue : La requête dépasse la taille maximale que le serveur est configuré pour accepter.
curl -v -X POST http://127.0.0.1:8080 -d "$(head -c 1000000 /dev/urandom | base64)"

2. 401 Unauthorized (401 - Non autorisé)
Situations possibles :

Authentification requise : Si vous implémentez une protection par authentification pour certaines ressources, le serveur renverra ce code lorsque le client n'a pas fourni de credentials valides.
curl -v http://127.0.0.1:8080/protected-resource

Accès à des ressources protégées : Le client tente d'accéder à une page ou un fichier nécessitant une authentification, mais les informations d'authentification sont manquantes ou invalides.
curl -v http://127.0.0.1:8080/protected-resource --user fakeuser:fakepassword

3. 403 Forbidden (403 - Accès interdit)
Situations possibles :

Permissions de fichier insuffisantes : Les permissions du système de fichiers empêchent le serveur de lire le fichier demandé, même si le chemin est correct.
chmod 000 /chemin/vers/webroot/secret.html
curl -v http://127.0.0.1:8080/secret.html

Interdiction de lister le contenu : Si l'autoindex est désactivé pour un répertoire et qu'il n'y a pas de fichier d'index (comme index.html), le serveur peut renvoyer une erreur 403.
curl -v http://127.0.0.1:8080/private-directory/

4. 404 Not Found (404 - Page non trouvée)
Situations possibles :

Ressource inexistante : Le client demande une page, un fichier ou une ressource qui n'existe pas sur le serveur.
curl -v http://127.0.0.1:8080/nonexistent-page.html

Chemin incorrect : L'URL fournie par le client est incorrecte ou contient des erreurs de frappe.
curl -v http://127.0.0.1:8080/wrongpath/

5. 405 Method Not Allowed (405 - Méthode non autorisée)
Situations possibles :

Méthode HTTP non supportée : Le client utilise une méthode HTTP (comme PUT, DELETE, PATCH) que le serveur ne reconnaît pas ou n'accepte pas pour la ressource demandée.
curl -v -X TRACE http://127.0.0.1:8080/

Restriction sur les méthodes : Le serveur est configuré pour n'accepter que certaines méthodes pour une ressource donnée. Par exemple, une page qui n'accepte que GET et HEAD, et le client envoie une requête POST.
curl -v -X DELETE http://127.0.0.1:8080/index.html

6. 408 Request Timeout (408 - Délai d'attente dépassé)
Situations possibles :

Inactivité du client : Le client met trop de temps à envoyer l'intégralité de sa requête, et le serveur ferme la connexion après un certain délai.

Délai dépassé lors du téléchargement : Lors du téléversement de fichiers, si le client met trop de temps à envoyer les données, le serveur peut renvoyer cette erreur.

7. 500 Internal Server Error (500 - Erreur interne du serveur)
Situations possibles :

Erreur dans le code serveur : Une exception non gérée ou un bug dans le code du serveur provoque une panne lors du traitement de la requête.

Échec du CGI : Le script Python exécuté via execve plante, retourne une erreur ou produit une sortie non conforme, ce qui empêche le serveur de traiter la réponse correctement.

Problème de permissions : Le serveur n'a pas les permissions nécessaires pour exécuter le script CGI ou accéder à une ressource requise.

8. 501 Not Implemented (501 - Fonctionnalité non implémentée)
Situations possibles :

Méthode non implémentée : Le client utilise une méthode HTTP que le serveur ne reconnaît pas du tout, comme TRACE ou CONNECT, et le serveur n'a pas de traitement pour cette méthode.

Fonctionnalité non supportée : Le client demande une fonctionnalité que le serveur ne supporte pas, comme une certaine version du protocole HTTP.

9. 502 Bad Gateway (502 - Mauvaise passerelle)
Situations possibles :

Échec de communication avec le CGI : Si votre serveur agit comme une passerelle vers le script CGI et que la communication échoue (par exemple, le script ne se lance pas correctement), vous pourriez renvoyer cette erreur.

Sortie invalide du CGI : Le script CGI renvoie une réponse mal formée que le serveur ne peut pas interpréter.

10. 503 Service Unavailable (503 - Service indisponible)
Situations possibles :

Maintenance du serveur : Le serveur est temporairement hors service pour maintenance.

Surcharge du serveur : Le serveur est trop occupé pour traiter la requête en raison d'un trop grand nombre de connexions simultanées.

Limitation de ressources : Les ressources système (comme la mémoire ou le CPU) sont insuffisantes pour traiter la requête.

11. 504 Gateway Timeout (504 - Délai d'attente de la passerelle)
Situations possibles :

Timeout avec le CGI : Le script Python appelé via execve met trop de temps à répondre, et le serveur atteint le délai maximum configuré pour attendre une réponse.

Problème de communication inter-processus : Si le serveur attend une réponse d'un processus externe (comme le CGI) et que cette réponse n'arrive pas à temps.

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
