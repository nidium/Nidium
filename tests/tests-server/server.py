#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

#   Copyright 2016 Nidium Inc. All rights reserved.
#   Use of this source code is governed by a MIT license
#   that can be found in the LICENSE file.

import sys

from twisted.internet import reactor
from twisted.python import log
from twisted.web.resource import Resource
from twisted.web import server
from twisted.web.server import Site
from twisted.web import static

from autobahn.twisted.websocket import WebSocketServerFactory, \
    WebSocketServerProtocol

from autobahn.twisted.resource import WebSocketResource

class EchoServerProtocol(WebSocketServerProtocol):
    def onConnect(self, req):
        print("WebSocket connection req: {}".format(req))

    def onMessage(self, payload, isBinary):
        self.sendMessage(payload, isBinary)

class HTTPTests(Resource):
    def __init__(self, path):
        Resource.__init__(self)
        self.test = path.split("?", 1)[0]

    def render_GET(self, req):
        return self._process(req)

    def render_POST(self, req):
        return self._process(req)

    def testNotFound(self, req):
        req.setResponseCode(404)
        return "No tests named \"%s\"" % (self.test)

    def _process(self, req):
        return getattr(self, "test_%s" % (self.test.replace("-", "_")), self.testNotFound)(req)

    def test_hello(self, req):
        return "Hello World !"

    def test_json(self, req):
        req.setHeader('Content-Type', 'application/json')
        return '{"hello":"world"}'

    def test_echo(self, req):
        if req.method == "GET":
            data = req.uri
        else:
            data = req.content.read()

        for name, value in req.getAllHeaders().items():
            if name.lower() == "content-length":
                name = "request-content-length"
            req.setHeader(name, value)

        req.write(data)
        req.finish()

        return server.NOT_DONE_YET

    def test_404(self, req):
        req.setResponseCode(404)
        return ""

    def test_toosmall_content_length(self, req):
        req.setResponseCode(200)
        req.setHeader("content-length", 1)
        req.write("This string is definitely longer than 1 character")
        req.finish()
        return server.NOT_DONE_YET

    def test_toolarge_content_length(self, req):
        req.setResponseCode(200)
        req.setHeader("content-length", 1000000000000000000000000000000000000000000000)
        req.write("This string is smaller than 100 characters")
        req.finish()
        return server.NOT_DONE_YET

    def test_no_content_length(self, req):
        req.setResponseCode(200)
        req.setHeader("Content-type", "text/html")
        req.write("This request does not have any content-length header")
        req.finish()
        return server.NOT_DONE_YET

    def test_utf8(self, req):
        req.setHeader('Content-type','text/html; charset=utf-8')
        return "♥ nidium ♥"

    def test_iso(self, req):
        req.setHeader('Content-type','text/html; charset=ISO-8859-15')
        return "\xE9"

    def test_progress(self, req, sendLength=True):
        def done(req):
            req.write("b"*2048)
            req.finish()

        req.setResponseCode(200)

        if sendLength:
            req.setHeader("content-length", 4096)

        req.write("a"*2048)
        reactor.callLater(0.500, done, req)
        return server.NOT_DONE_YET

    def test_progress_no_content_length(self, req):
        return self.test_progress(req, sendLength=False)

    def test_empty_content_type(self, req):
        req.setHeader('Content-Type', "")
        req.setResponseCode(200)
        req.write("This request has an empty content-type")
        req.finish()
        return server.NOT_DONE_YET

    def test_image(self, req):
        req.setHeader("Content-Type", "image/png")
        return open("./nidium_32x32.png").read()

class HTTPDispatcher(Resource):
    def getChild(self, path, req):
        req.setHeader("Server", "Nidium tests server")
        return HTTPTests(path)

class HTTPRoot(Resource):
    def render_GET(self, req):
        return "Hello, this is nidium tests server"

    def render_POST(self, req):
        return "Hello, this is nidium tests server"

def start(port=8888, useSSL=False, sslPort=8443):
    log.startLogging(sys.stdout)

    wsFactory = WebSocketServerFactory()
    wsFactory.protocol = EchoServerProtocol

    root = HTTPRoot()
    root.putChild(b"ws", WebSocketResource(wsFactory))
    root.putChild(b"http", HTTPDispatcher())

    if useSSL:
        from twisted.internet import ssl
        sslContext = ssl.DefaultOpenSSLContextFactory("localhost.key", "localhost.cert")
        reactor.listenSSL(sslPort, Site(root), contextFactory=sslContext)

    reactor.listenTCP(port, Site(root))

    reactor.run()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Nidium HTTP/WebSocket tests server')
    parser.add_argument('--port', action="store", dest="port", type=int, default=8888)
    parser.add_argument('--ssl-port', action="store", dest="sslPort", type=int, default=8443)
    parser.add_argument('--ssl', action="store_true", dest="ssl", default=False)

    args = parser.parse_args()

    start(port=args.port, useSSL=args.ssl, sslPort=args.sslPort)
