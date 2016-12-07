#!/bin/bash

#   Copyright 2016 Nidium Inc. All rights reserved.
#   Use of this source code is governed by a MIT license
#   that can be found in the LICENSE file.


DOMAINS="tests.nidium.com"

cd /home/nidium/certbot/
./certbot-auto certonly --expand --no-self-upgrade --noninteractive --webroot -w /home/nidium/certbot/www/ --agree-tos --email "efyx.ps@gmail.com" -d $DOMAINS
