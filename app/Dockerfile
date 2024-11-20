FROM nginx:latest

# Installer fcgiwrap, PHP et PHP-FPM
# RUN apt-get update && apt-get install -y fcgiwrap spawn-fcgi php-fpm
RUN apt-get update && apt-get install -y fcgiwrap spawn-fcgi

# Copier le fichier de configuration NGINX
COPY nginx.conf /etc/nginx/nginx.conf

# Copier les fichiers du site web, y compris les scripts CGI
COPY www /usr/share/nginx/html

# Donner les bonnes permissions pour exécuter les scripts CGI
RUN chmod +x /usr/share/nginx/html/cgi-bin/*

# Démarrer fcgiwrap avec spawn-fcgi et NGINX
# CMD spawn-fcgi -s /var/run/fcgiwrap.socket -M 766 /usr/sbin/fcgiwrap && nginx -g 'daemon off;'


# Démarrer fcgiwrap avec spawn-fcgi et NGINX (la syntaxe Json permet une meilleure gestion des signaux)
CMD ["sh", "-c", "spawn-fcgi -s /var/run/fcgiwrap.socket -M 766 /usr/sbin/fcgiwrap && nginx -g 'daemon off;'"]

# Démarrer PHP-FPM et fcgiwrap avec spawn-fcgi et NGINX
# CMD ["sh", "-c", "service php7.4-fpm start && spawn-fcgi -s /var/run/fcgiwrap.socket -M 766 /usr/sbin/fcgiwrap && nginx -g 'daemon off;'"]
