#!/bin/bash

BASE_NGINX_SSL_LISTEN="listen 8443 ssl;"

function updateNginxConf {
    cat /home/nidium/nginx/nginx.template.conf | envsubst "\$NGINX_SSL_CERT \$NGINX_SSL_LISTEN" > /usr/local/nginx/conf/nginx.conf
}

if [ -z "$LOCAL_TESTS" ]; then
    CERT_FILE=/etc/letsencrypt/live/tests.nidium.com/cert.pem
    BASE_NGINX_SSL_CERT="ssl_certificate /etc/letsencrypt/live/tests.nidium.com/fullchain.pem;ssl_certificate_key /etc/letsencrypt/live/tests.nidium.com/privkey.pem;"

    if [ ! -f "$CERT_FILE" ]
    then
        echo "=> Generating letsentrycpt certificates"

        echo "=> Starting nginx without SSL for certbot verification"
        export NGINX_SSL_LISTEN=""
        export NGINX_SSL_CERT=""

        updateNginxConf
        /usr/local/nginx/sbin/nginx &
        sleep 3

        echo "=> Running certbot"
        /home/nidium/certbot/certbot.sh
        if [ $? -ne 0 ]; then
            echo "=> FAILED TO SETUP LETSENCRYPT CERTIFICATE"
            exit 2
        fi
        echo "=> Stopping nginx"
        /usr/local/nginx/sbin/nginx -s stop
    else
        echo "=> Checking letsencrypt certificate"
        if openssl x509 -checkend $((3600*24*10)) -noout -in $CERT_FILE
        then
          echo "=> Certificate is good for at least 10 more days"
        else
          echo "=> Certificate has expired or will do so within 10 days !"
        fi
        openssl x509 -enddate -noout -in $CERT_FILE
    fi
else
    BASE_NGINX_SSL_CERT="ssl_certificate /home/nidium/nginx/localhost.cert;ssl_certificate_key /home/nidium/nginx/localhost.key;"
fi

# Enable certificate
export NGINX_SSL_LISTEN=$BASE_NGINX_SSL_LISTEN
export NGINX_SSL_CERT=$BASE_NGINX_SSL_CERT
updateNginxConf

echo "=> Starting cron"
cron

echo "=> Starting tests servers"
/home/nidium/tests/server.py --port 8000 &

echo "=> Starting nginx"
/usr/local/nginx/sbin/nginx
