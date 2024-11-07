#include "../includes/HttpResponse.hpp"
#include "../includes/Color_Macros.hpp"
#include <sstream>
#include <iostream>

/*
    classe qui contient les attributs necessaires a la construction d' une reponse http
    cette classe est utilisee par requestHandler qui va set tous ses attributs puis HttpResponse::generateResponse() genere la reponse sous forme de string

*/

HttpResponse::HttpResponse()
    : statusCode(200), reasonPhrase("OK"), body("") {
    headers["Content-Type"] = "text/html";
}

void HttpResponse::setStatusCode(int code) {
    statusCode = code;
    reasonPhrase = getDefaultReasonPhrase(code);
}

void HttpResponse::setReasonPhrase(const std::string& phrase) {
    reasonPhrase = phrase;
}

void HttpResponse::setBody(const std::string& bodyContent) {
    body = bodyContent;
    // Update Content-Length header
    std::ostringstream oss;
    oss << body.size();
    headers["Content-Length"] = oss.str();
}

void HttpResponse::setHeader(const std::string& headerName, const std::string& headerValue) {
    headers[headerName] = headerValue;
}

std::string HttpResponse::generateResponse() const {
    // std::cout << YELLOW <<"\n\n\n\nREPONSE :HttpResponse::generateResponse()\n"<< std::endl;//test
    std::ostringstream response;

    // Status line
    response << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    response << "\r\n"; // Empty line to separate headers from body

    // std::cout << "HEADERS\n" << response.str() << RESET << std::endl;//test
    // std::cout << "\nBODY\n" << body << RESET << std::endl;//test

    // Body
    response << body;

    return response.str();
}

std::string HttpResponse::getDefaultReasonPhrase(int code) const {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method not Allowed";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}
/*
1. Codes de succès (2xx)
Ces codes indiquent que la requête a été traitée avec succès.

200 OK : La requête a réussi (GET, POST, DELETE). Utilisé lorsque le serveur retourne une réponse réussie pour une requête GET, traite une requête POST correctement, ou supprime une ressource pour une requête DELETE.

201 Created : Utilisé lorsque une ressource a été créée avec succès à la suite d'une requête POST (par exemple, après le téléversement d'un fichier).

204 No Content : Utilisé lorsque la requête a réussi, mais qu'il n'y a pas de contenu à renvoyer dans la réponse. Courant pour des requêtes DELETE réussies.

206 Partial Content : Ce code est utilisé dans le cas où le client demande une partie d'une ressource (par exemple, une requête GET avec des en-têtes de range, utile pour des fichiers volumineux).

2. Redirections (3xx)
Ces codes indiquent que le client doit effectuer une action supplémentaire pour compléter la requête.

301 Moved Permanently : Utilisé si une ressource demandée a été déplacée de façon permanente. Le client est invité à utiliser l'URL fournie dans l'en-tête Location pour les futures requêtes.

302 Found : Utilisé pour rediriger temporairement une requête vers une autre URL.

304 Not Modified : Utilisé pour indiquer au client que la version en cache de la ressource est encore valide, donc il n'est pas nécessaire de renvoyer les données.

3. Erreurs du client (4xx)
Ces codes sont utilisés lorsque le client a fait une erreur dans sa requête.

400 Bad Request : La requête du client est mal formée ou invalide (par exemple, si des paramètres obligatoires sont manquants ou mal structurés).

401 Unauthorized : Utilisé lorsque l'accès à une ressource nécessite une authentification et que celle-ci n'a pas été fournie ou est incorrecte.

403 Forbidden : Le serveur comprend la requête, mais refuse de la traiter (par exemple, une requête DELETE sur une ressource protégée ou interdite).

404 Not Found : Utilisé lorsque la ressource demandée n'existe pas sur le serveur (cas d'un GET, POST ou DELETE sur une URL inexistante).

405 Method Not Allowed : Utilisé lorsque la méthode HTTP utilisée n'est pas autorisée pour la ressource demandée (par exemple, essayer de faire un POST sur une ressource qui ne supporte que GET).

413 Payload Too Large : Utilisé lorsque le client tente de téléverser un fichier ou des données dont la taille dépasse la limite autorisée par le serveur.

415 Unsupported Media Type : Le format du fichier téléversé n'est pas supporté par le serveur (par exemple, tenter d'envoyer un fichier d'un type non autorisé).

418 I'm a teapot : Un code d'état humoristique défini dans la RFC 2324 pour signifier que le serveur est un "théière" et ne peut pas faire du café. Peu utilisé, mais parfois présent comme clin d'œil dans certaines implémentations.

4. Erreurs du serveur (5xx)
Ces codes indiquent que le serveur a rencontré une erreur interne.

500 Internal Server Error : Utilisé lorsque le serveur rencontre une erreur inattendue lors du traitement de la requête (erreur dans le code CGI, problème d'exécution, etc.).

501 Not Implemented : Le serveur ne prend pas en charge la méthode HTTP utilisée (si, par exemple, DELETE n'est pas implémenté).

502 Bad Gateway : Le serveur, agissant comme un proxy ou une passerelle, a reçu une réponse invalide d'un autre serveur.

503 Service Unavailable : Le serveur est temporairement indisponible, généralement à cause d'une surcharge ou de travaux de maintenance.

504 Gateway Timeout : Utilisé lorsque le serveur, agissant en tant que proxy ou passerelle, n'a pas reçu de réponse à temps d'un serveur en amont.

5. Spécificités pour le téléversement de fichiers et CGI
Pour le téléversement de fichiers et les CGI, plusieurs codes sont particulièrement pertinents :

413 Payload Too Large : Si le fichier téléversé dépasse la taille maximale autorisée.
500 Internal Server Error : Si le script CGI échoue ou génère une erreur.
415 Unsupported Media Type : Si le fichier téléversé a un type MIME non accepté par le serveur.
201 Created : Si le fichier est téléversé avec succès.

*/
