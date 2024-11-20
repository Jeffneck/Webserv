#!/usr/bin/python3

import sys
import html

# Fonction pour analyser les arguments de la ligne de commande
def parse_arguments():
    args = sys.argv[1:]
    params = {}
    for arg in args:
        if arg.startswith('--'):
            key_value = arg[2:].split('=', 1)
            if len(key_value) == 2:
                key, value = key_value
                params[key] = value
    return params

# Récupérer les données du formulaire à partir des arguments
params = parse_arguments()

name = params.get('name')
email = params.get('email')
message = params.get('message')

# Vérifier que tous les champs sont remplis
if not all([name, email, message]):
    # print("Content-Type: text/html; charset=UTF-8")
    # print()
    print("<html><body>")
    print("<h1>Erreur</h1>")
    print("<p>Veuillez remplir tous les champs du formulaire.</p>")
    print("</body></html>")
    exit()

# Traitement des données (par exemple, les enregistrer dans un fichier)
try:
    with open('forms/contacts.txt', 'a', encoding='utf-8') as f:
        f.write(f"Nom: {name}\nEmail: {email}\nMessage: {message}\n---\n")
except Exception as e:
    # print("Content-Type: text/html; charset=UTF-8")
    # print()
    print("<html><body>")
    print("<h1>Erreur</h1>")
    print(f"<p>Une erreur est survenue lors de l'enregistrement des données: {e}</p>")
    print("</body></html>")
    exit()

# Afficher une page de confirmation
# print("Content-Type: text/html; charset=UTF-8")
# print()
print(f"""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Merci de nous avoir contactés</title>
</head>
<body>
    <h1>Merci, {html.escape(name)}</h1>
    <p>Votre message a été reçu avec succès. Nous vous contacterons bientôt à l'adresse {html.escape(email)}.</p>
    <p>Voici le recapitulatif de votre message :'{html.escape(message)}'.</p>
</body>
</html>
""")
