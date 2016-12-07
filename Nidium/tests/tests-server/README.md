# Nidium tests server

This directory contains nidium Socket/HTTP/WebSocket test server. It can be run trough Docker or in standalone mode

## Docker
Run locally (with self signed HTTPS certificate and using Nginx as a proxy): 
```
docker-compose up local_tests
```

Or in production mode (with letsencrypt HTTPS certificate):
```
docker-compose up tests
```

## Standalone
It's also possible to run the test server without Docker:
```
virtualenv .venv/
source .venv/bin/activate
pip install -r requirements.txt
python server.py --ssl
````
