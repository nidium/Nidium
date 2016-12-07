FROM debian:jessie

RUN apt-get -y update && apt-get -y install python2.7 python-pip git gettext-base openssl libpcre3 libpcre3-dev  zlib1g-dev libssl-dev cron python-dev curl libffi-dev

RUN useradd -ms /bin/bash nidium

RUN mkdir -p /home/nidium/nginx/src/
RUN mkdir /home/nidium/tests/

WORKDIR /home/nidium/

# Install nginx
RUN cd nginx/src/ && curl -O http://nginx.org/download/nginx-1.10.1.tar.gz && tar xzvf nginx-1.10.1.tar.gz
RUN cd nginx/src/nginx-1.10.1 && ./configure --with-http_ssl_module --with-pcre && make && make install

# Install python required packages
ADD requirements.txt /home/nidium/tests/requirements.txt
RUN pip install -r /home/nidium/tests/requirements.txt

# Add default certificates (for localhost testing)
ADD localhost.cert /home/nidium/nginx
ADD localhost.key /home/nidium/nginx/
ADD dhparams.pem /home/nidium/nginx/

# {{{ Letsencrypt / certbot setup 
RUN git clone https://github.com/certbot/certbot.git
RUN cd certbot && ./certbot-auto --noninteractive --os-packages-only
ADD certbot.sh certbot/certbot.sh
RUN mkdir /home/nidium/certbot/www/
RUN chown -R www-data:www-data /home/nidium/certbot/www/

# Add volume for storing letsencrypt keys
VOLUME /etc/letsencrypt/
# }}}

EXPOSE 8888 8443

# Nginx startup script & configuration
ADD start.sh /home/nidium/start.sh
RUN chmod +x /home/nidium/start.sh

ADD nginx.conf /home/nidium/nginx/nginx.template.conf

CMD /home/nidium/start.sh
