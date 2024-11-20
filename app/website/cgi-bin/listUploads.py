#!/usr/bin/python3
import os
import cgi
import cgitb
import mimetypes

# Activer le débogage en cas d'erreur
cgitb.enable()

# Dossier contenant les fichiers multimédias
dir_path = '../uploads/'

try:
    # Récupérer tous les fichiers du dossier
    files = os.listdir(dir_path)
except Exception as e:
    print("Content-Type: text/plain; charset=UTF-8")
    print()
    print(f"Impossible d'ouvrir le dossier '{dir_path}': {e}")
    exit()

# Début du document HTML
# print("Content-Type: text/html; charset=UTF-8")
# print()
print("""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Galerie Multimédia</title>
    <style>
        .media-container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
        }
        .media-item {
            margin: 10px;
            text-align: center;
        }
        img, video, audio {
            max-width: 200px;
            max-height: 200px;
        }
    </style>
</head>
<body>
<h1 style='text-align: center;'>Galerie Multimédia</h1>
<div class='media-container'>
""")

# Traiter chaque fichier
for file in files:
    if file.startswith('.'):
        continue  # Ignorer . et ..

    ext = os.path.splitext(file)[1].lower()

    print("<div class='media-item'>")

    if ext in ['.jpg', '.jpeg', '.png', '.gif']:
        print(f"<img src='{dir_path}{file}' alt='{file}'>")
    elif ext in ['.mp4', '.webm']:
        mime_type, _ = mimetypes.guess_type(file)
        print(f"""<video controls>
                    <source src='{dir_path}{file}' type='{mime_type}'>
                    Votre navigateur ne prend pas en charge la lecture de vidéos.
                </video>""")
    elif ext in ['.mp3', '.wav']:
        mime_type, _ = mimetypes.guess_type(file)
        print(f"""<audio controls>
                    <source src='{dir_path}{file}' type='{mime_type}'>
                    Votre navigateur ne prend pas en charge l'audio.
                </audio>""")
    print(f"<p>{file}</p>")
    print("</div>")

print("""
</div>
</body>
</html>
""")
