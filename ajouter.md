Ajouter un request timeout (err 408) si le client met trop de temps a televerser un fichier prob
void flushSocket(int sockfd) {
    char buffer[1024];  // Buffer temporaire pour lire les données

    // Lire et ignorer les données présentes dans le tampon du socket
    while (recv(sockfd, buffer, sizeof(buffer), MSG_DONTWAIT) > 0) {
        // Continue à lire tant qu'il y a des données dans le tampon
    }
}

Retour d' erreur du cgi pas recup

rechercher http://127.0.0.1:8080/cgi-bin/a.py provoque err 500 au lieu de 404

lorsque de nombreux serveurs, on dirait que getRoot n' est pas pris en compte

verif les .py dans autre chose que la bonne location

Dans le checker de parsing il faut verifier qu' il y a au moins 1 serveur et qu' il a une directive listen et root

televerser des files est bloquant

PATCHED comprendre pourquoi l' utilisation de la memoire augmente constamment (ps.log pdt le stress_test)

verifier que les codes qui ne sont pas des codes d' erreurs sont respectes

quel code d' erreur en cas de retour d' erreur cgi ?
quel code d' erreur si autoindex ne parvient pas a ouvrir le directory


trop grosse requete > client max body size
trop grosse requete televersement

fichier de config vide

listen pas dans config

verifier les sockets ouverts en fin de process


tester de rendre non executable les scripts cgi et les fichiers static

verifier ce qu' il se passe si trop de clients demandent en simultane a mon serveur ?

erreur du meme port set plusieurs fois sans hostname