Checker avec valgrind : 
valgrind --leak-check=full ./test_webserv > stdout 2>stderr

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


Voici quelques requêtes supplémentaires que tu peux tester pour t'assurer que ton serveur respecte bien les spécifications de ton devoir :

1. Test des pages d'erreur personnalisées :
Crée une route qui renvoie une page d'erreur personnalisée pour un code d'état spécifique, comme 404.
curl -I http://127.0.0.1:8080/nonexistent
2. Téléchargement de fichiers :
Vérifie que ton serveur permet de télécharger des fichiers correctement.
curl -O http://127.0.0.1:8080/fichier_a_telecharger.txt
3. Téléversement de fichiers (POST) :
Teste l'envoi de fichiers à ton serveur avec une requête POST.
curl -X POST http://127.0.0.1:8080/upload -F "file=@chemin/vers/fichier.txt"
4. Limitation de la taille du body des requêtes :
Envoie une requête avec un body plus grand que la taille maximale configurée.
curl -X POST http://127.0.0.1:8080 -d "data=$(head -c 1000000 /dev/urandom | base64)"
5. Test des routes et redirections :
Configure une route avec une redirection HTTP et teste-la.
curl -I http://127.0.0.1:8080/redirection
6. Liste de répertoires activée/désactivée :
Crée une route où le listing des fichiers d'un répertoire est autorisé, puis teste l'affichage de la liste.
curl http://127.0.0.1:8080/repertoire
7. Exécution de CGI (par exemple, un script PHP ou Python) :
Vérifie que le CGI est bien exécuté pour certains fichiers et extensions.
curl http://127.0.0.1:8080/script.php
8. Vérification des méthodes HTTP supportées par une route :
Configure une route pour qu'elle accepte uniquement certaines méthodes (par exemple, GET et POST) et teste le rejet d'autres méthodes.
curl -X DELETE http://127.0.0.1:8080/restricted_route
9. Multi-port :
Configure ton serveur pour écouter sur plusieurs ports et teste les connexions sur chaque port.
curl http://127.0.0.1:8081
10. Stress test :
Lance un test de charge pour vérifier la résilience de ton serveur.
ab -n 1000 -c 100 http://127.0.0.1:8080/
(Cette commande utilise ab - Apache Benchmark - pour générer des requêtes simultanées.)



Voici une série de tests pour vérifier la gestion des erreurs HTTP 1.1 par ton serveur, en se basant sur les codes d'état courants et requis par la norme. Tu peux utiliser curl pour simuler chaque situation et observer les réponses de ton serveur.

1. 400 Bad Request
Requête malformée ou incorrecte.
curl -v -X GET http://127.0.0.1:8080 -H "Host:"  # Header Host vide ou mal formé
2. 401 Unauthorized
Accès à une ressource protégée sans authentification.
curl -v http://127.0.0.1:8080/protected_resource
(Configure une ressource sur ton serveur qui nécessite une authentification).

3. 403 Forbidden
Accès à une ressource interdite.
curl -v http://127.0.0.1:8080/forbidden_directory
(Configure une route pour interdire l'accès à certains fichiers ou répertoires).

4. 404 Not Found
Page ou ressource inexistante.
curl -v http://127.0.0.1:8080/nonexistent_resource
5. 405 Method Not Allowed
Requête avec une méthode HTTP non acceptée sur une route.
curl -v -X DELETE http://127.0.0.1:8080/get_only_route
(Configure une route qui accepte uniquement la méthode GET par exemple).

6. 408 Request Timeout
Requête trop longue ou incomplète (simulateur de timeout).
curl -v --max-time 1 http://127.0.0.1:8080
(Il faut que ton serveur gère les délais d'attente pour renvoyer cette erreur).

7. 413 Payload Too Large
Envoi d'un body dépassant la limite configurée.
curl -v -X POST http://127.0.0.1:8080 -d "$(head -c 10000000 /dev/urandom | base64)"  # Fichier très volumineux
(Configure une limite de taille de body dans ton fichier de configuration).

8. 414 URI Too Long
URL trop longue pour être traitée par le serveur.
curl -v http://127.0.0.1:8080/$(python -c 'print("a"*5000)')
(Le chemin de l'URL est trop long pour être traité).

9. 415 Unsupported Media Type
Type de contenu non pris en charge.
curl -v -X POST http://127.0.0.1:8080/upload -H "Content-Type: application/unsupported" -d "test"
(Configure ton serveur pour accepter certains types de contenu et renvoyer cette erreur pour les autres).

10. 500 Internal Server Error
Erreur interne du serveur.
curl -v http://127.0.0.1:8080/trigger_500_error
(Tu peux simuler cette erreur en créant un bug volontaire ou un cas de CGI mal configuré).

11. 501 Not Implemented
Méthode HTTP non implémentée.
curl -v -X PUT http://127.0.0.1:8080
(Si tu n'as pas implémenté la méthode PUT, ton serveur devrait renvoyer cette erreur).

12. 502 Bad Gateway
Erreur lors de la communication avec un backend (par exemple, un CGI défectueux).
curl -v http://127.0.0.1:8080/cgi_error
(Simule un cas où ton CGI échoue à répondre correctement).

13. 503 Service Unavailable
Le serveur est temporairement indisponible.
curl -v http://127.0.0.1:8080
(Simule une surcharge ou une maintenance planifiée sur ton serveur).

14. 504 Gateway Timeout
Timeout lors de l'attente d'une réponse d'un backend ou CGI.
curl -v http://127.0.0.1:8080/cgi_slow_response
(Simule une lenteur excessive dans la réponse du CGI).

15. 505 HTTP Version Not Supported
Version HTTP non supportée.
curl -v --http1.0 http://127.0.0.1:8080
(Si ton serveur est exclusivement HTTP 1.1, il doit refuser les requêtes HTTP 1.0).

