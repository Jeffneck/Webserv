#!/usr/bin/python3

import os
import sys

def main():
    # Obtenir le répertoire de travail courant
    current_working_directory = os.getcwd()

    # Obtenir les arguments passés au script
    script_arguments = sys.argv

    # Obtenir l'environnement du script
    environment_vars = os.environ

    # Afficher les informations comme réponse CGI
    # print("Content-Type: text/html\n")  # En-tête CGI indiquant le type de contenu (HTML)

    # Générer le contenu HTML
    print("<html>")
    print("<head><title>Information CGI</title></head>")
    print("<body>")
    print("<h1>Information sur le script CGI</h1>")
    
    # Afficher le répertoire de travail
    print(f"<p>Le repertoire de travail actuel est : <strong>{current_working_directory}</strong></p>")
    
    # Afficher les arguments passés au script
    print("<h2>Arguments passes au script :</h2>")
    if len(script_arguments) > 1:
        print("<ul>")
        for i, arg in enumerate(script_arguments):
            print(f"<li>Argument {i}: {arg}</li>")
        print("</ul>")
    else:
        print("<p>Aucun argument recu.</p>")
    
    # Afficher les variables d'environnement
    print("<h2>Variables d'environnement :</h2>")
    print("<ul>")
    for key, value in environment_vars.items():
        print(f"<li>{key} = {value}</li>")
    print("</ul>")
    
    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()


# Lecture de stdin :

# sys.stdin.read() permet de lire tout ce qui est envoyé au script via stdin, souvent utilisé pour les requêtes POST en CGI.
# Récupération des arguments :

# sys.argv contient tous les arguments passés au script. Le premier élément est le chemin du script lui-même, les suivants sont les arguments supplémentaires.
# Affichage des variables d'environnement :

# os.environ est un dictionnaire contenant toutes les variables d'environnement disponibles au moment de l'exécution. Chaque clé est le nom de la variable d'environnement, et chaque valeur est son contenu.
# Résultat attendu :
# Lorsque vous exécutez ce script via CGI sur votre serveur, il affichera :

# Le répertoire de travail.
# Toutes les données reçues via stdin (par exemple, le corps d'une requête POST).
# Les arguments passés au script (souvent utilisés pour traiter des paramètres dans une URL).
# Toutes les variables d'environnement du script, ce qui est particulièrement utile pour voir les informations transmises par le serveur (comme REQUEST_METHOD, CONTENT_LENGTH, etc.).
# Exécution :
# Placez ce script dans le répertoire cgi-bin de votre serveur.
# Rendez-le exécutable :
# bash
# Copier le code
# chmod +x /var/www/html/cgi-bin/votre_script.py
# Testez-le en accédant à son URL via votre navigateur ou en envoyant des requêtes avec des données via des outils comme curl ou Postman.
# Cela vous aidera à voir en détail tout ce que le script reçoit et comment il est exécuté dans le contexte de votre serveur CGI.