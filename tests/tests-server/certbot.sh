#!/bin/bash

DOMAINS="tests.nidium.com"

cd /home/nidium/certbot/
./certbot-auto certonly --expand --no-self-upgrade --noninteractive --webroot -w /home/nidium/certbot/www/ --agree-tos --email "efyx.ps@gmail.com" -d $DOMAINS
