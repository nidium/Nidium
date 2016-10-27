#!/usr/bin/env python
# -*- coding: utf-8 -*-

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

import sys, random, os
import time

Protocol     = "HTTP/1.0"

class myHandlerClass(BaseHTTPRequestHandler):
    #Handler for the GET requests
    def do_POST(self):
        self._process("post")

    def do_GET(self):
        self._process("get")

    def _process(self, method):
        if self.path == "/kill":
            self.send_response(200)
            self.end_headers()
            self.server.stop = True
        elif self.path == "/json":
            self.send_response(200)
            self.send_header('Content-type','application/json')
            self.end_headers()
            self.wfile.write('{"hello":"world"}')
        elif self.path.startswith("/echo"):
            self.send_response(200)

            if method == "get":
                data = self.path
            else:
                data = self.rfile.read(int(self.headers.getheader('content-length', 0)))

            self.send_header("Content-type", self.headers.getheader("content-type", "text/html"))

            for name in self.headers:
                if name.lower() == "content-length":
                    name = "request-content-length"
                self.send_header(name, self.headers.get(name))


            self.send_header('Content-Length', len(data))
            self.end_headers()

            self.wfile.write(data)
        elif self.path == "/404":
            self.send_response(404)
            self.end_headers()
        elif self.path == "/toosmall-content-length":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.send_header('Content-Length',str(random.randrange(0, 10, 1)))
            self.end_headers()
            self.wfile.write("client will probably chocke on this")
        elif self.path == "/toolarge-content-length":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.send_header('Content-Length',str(random.randrange(0, 1000000000, 1)))
            self.end_headers()
            self.wfile.write("client will probably chocke on this")
        elif self.path == "/no-content-length":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write("")
        elif self.path == "/utf8":
            self.send_response(200)
            self.send_header('Content-type','text/html; charset=ISO-8859-1')
            self.end_headers()
            self.wfile.write("♥ nidium ♥")
        elif self.path == "/iso":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write("\xE9")
        elif self.path == "/progress":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.send_header('Content-Length', 10)
            self.end_headers()
            self.wfile.write("Hello")
            time.sleep(0.25);
            self.wfile.write("World")
        elif self.path == "/progress-no-content-length":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write("Hello")
            time.sleep(0.25);
            self.wfile.write("World")
        elif self.path == "/no-content-type":
            self.send_response(200)
            self.end_headers()
            self.wfile.write("Hello")
        else:
            text = "Hello World !"
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.send_header('Content-Length', len(text))
            self.end_headers()
            self.wfile.write(text)
        return

class StoppableHttpServer (HTTPServer):
    def serve_forever (self):
        self.stop = False
        while not self.stop:
            self.handle_request()
    
def serve(port):
    try:
        server_address = ('0.0.0.0', port)
        myHandlerClass.protocol_version = Protocol
        httpd = StoppableHttpServer(server_address, myHandlerClass)

        sa = httpd.socket.getsockname()
        print "Serving HTTP on", sa[0], "port", sa[1], "..."
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("^C received, shutting down webserver")
        server.socket.close()

if __name__ == '__main__':
    argc = len(sys.argv)
    fork = False
    port = 8000

    if sys.argv[argc - 1] == "--fork":
        fork = True
        argc -= 1

    if argc > 1:
        port = int(sys.argv[argc])

    if fork:
        pid = os.fork();
        if pid == 0:
            try:
                serve(port)
            except Exception as e:
                if "already in use" in str(e):
                    print "Looks like server is already running or port is used by another process"
                    sys.exit(1)
                else:
                    import traceback
                    print "Failed to run server " + str(e)
                    traceback.print_exc()
                    sys.exit(2)
        else:
            print "%s" % pid
    else:
        serve(port)

