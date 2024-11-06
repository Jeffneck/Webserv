# creer un nouveau fichier de config
sudo vim /etc/nginx/sites-available/new.conf

# deplacer un fichier de config webserv vers nginx
sudo cp ~/Desktop/webserv/ALL2/config/file.conf /etc/nginx/sites-available/file.conf

# inverse 
sudo cp  /etc/nginx/sites-available/simpleWebsite.conf ~/Desktop/webserv/ALL2/nginxApp 

# deplacer mon site vers nginx
sudo  cp -r ~/Desktop/webserv/ALL2/site /var/www/site

# inverse
sudo cp -r   /var/www/SimpleWebsite  ~/Desktop/webserv/ALL2/nginxApp/

# activer une config et redemarrer nginx
sudo ln -s /etc/nginx/sites-available/simpleWebsite.conf /etc/nginx/sites-enabled/
sudo nginx -t  # Vérifiez la syntaxe de la configuration
sudo systemctl restart nginx  # Redémarrez Nginx
sudo systemctl start nginx

# lancer le docker sans dockercompose
<!-- docker run --name my-nginx -p 8080:8080 -v $(pwd)/SimpleWebsite.conf:/etc/nginx/nginx.conf -v $(pwd)/SimpleWebsite:/usr/share/nginx/html:ro -d nginx -->
docker build -t my-nginx-cgi .
docker run --name my-nginx-cgi -p 8080:8080 -d my-nginx-cgi

# afficher les dockers en cours 
docker ps 
docker ps -a

# stopper le docker et le supprimer 
docker stop my-nginx-cgi
docker rm my-nginx-cgi 