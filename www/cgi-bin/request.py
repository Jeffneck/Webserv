#!/usr/bin/python3

import os
import sys

def handle_get():
    # Pour une requête GET, on récupère les paramètres de la requête dans QUERY_STRING
    query_string = os.environ.get("QUERY_STRING", "")
    
    # Affichage des informations pour GET
    print("Content-Type: text/plain\n")  # En-tête CGI
    print("Méthode : GET")
    if query_string:
        print(f"Paramètres de la requête : {query_string}")
    else:
        print("Aucun paramètre fourni.")

def handle_post():
    # Pour une requête POST, on récupère la longueur du contenu à partir de CONTENT_LENGTH
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    
    # Lire le corps de la requête POST (les données envoyées)
    post_data = sys.stdin.read(content_length) if content_length > 0 else ""

    # Affichage des informations pour POST
    print("Content-Type: text/plain\n")  # En-tête CGI
    print("Méthode : POST")
    if post_data:
        print(f"Données reçues dans le corps de la requête : {post_data}")
    else:
        print("Aucune donnée envoyée dans la requête POST.")

def main():
    # Récupérer la méthode de la requête (GET ou POST)
    request_method = os.environ.get("REQUEST_METHOD", "")

    # Différencier le traitement en fonction de la méthode
    if request_method == "GET":
        handle_get()
    elif request_method == "POST":
        handle_post()
    else:
        # Si ce n'est ni GET ni POST, on affiche un message d'erreur
        print("Content-Type: text/plain\n")  # En-tête CGI
        print(f"Méthode HTTP non supportée : {request_method}")

if __name__ == "__main__":
    main()


# Explication du script :
# Méthode GET :

# La méthode GET récupère les paramètres de la requête à partir de QUERY_STRING. Ce sont les données qui se trouvent dans l'URL après le "?".
# Si des paramètres sont présents, ils sont affichés. Sinon, le script indique qu'il n'y a pas de paramètres.
# Méthode POST :

# La méthode POST reçoit les données dans le corps de la requête. Le script lit le nombre de caractères à recevoir à partir de la variable d'environnement CONTENT_LENGTH et récupère les données via sys.stdin.read().
# Si des données sont présentes dans le corps, elles sont affichées. Sinon, le script indique qu'aucune donnée n'a été envoyée.
# Autres méthodes HTTP :

# Si la requête utilise une méthode autre que GET ou POST, le script renvoie un message d'erreur indiquant que la méthode n'est pas supportée.
# Résultat attendu :
# Requête GET : Si vous accédez à ce script avec une URL comme http://votre_serveur/cgi-bin/votre_script.py?nom=John&age=30, le script affichera :

# sql
# Copier le code
# Méthode : GET
# Paramètres de la requête : nom=John&age=30
# Requête POST : Si vous envoyez une requête POST à ce script avec un corps (par exemple, nom=John&age=30), le script affichera :

# Copier le code
# Méthode : POST
# Données reçues dans le corps de la requête : nom=John&age=30
# Exécution :
# Placez ce script dans le répertoire cgi-bin de votre serveur.
# Assurez-vous qu'il est exécutable :
# bash
# Copier le code
# chmod +x /var/www/html/cgi-bin/votre_script.py
# Testez le script via un navigateur ou des outils comme curl ou Postman pour envoyer des requêtes GET et POST.
# Exemple de test avec curl pour une requête POST :
# bash
# Copier le code
# curl -X POST -d "nom=John&age=30" http://votre_serveur/cgi-bin/votre_script.py
# Cela affichera les données reçues dans le corps de la requête POST.