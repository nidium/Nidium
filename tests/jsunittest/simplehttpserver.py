#!/usr/bin/env python

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

import sys, random

Protocol     = "HTTP/1.0"

class myHandlerClass(BaseHTTPRequestHandler):
	#Handler for the GET requests
	def do_GET(self):
		if self.path == "/toosmall-content-length":
			self.send_response(200)
			self.send_header('Content-type','text/html')
			self.send_header('Content-Length',str(random.randrange(0, 10, 1)))
			self.wfile.write("client will probably chocke on this")
		elif self.path == "/toolarge-content-length":
			self.send_response(200)
			self.send_header('Content-type','text/html')
			self.send_header('Content-Length',str(random.randrange(20, 10000000, 1)))
			self.wfile.write("client will probably chocke on this")
		else:
			self.send_response(200)
			self.send_header('Content-type','text/html')
			self.end_headers()
			self.wfile.write("Hello World !")
		return
	

def serve(port):
	try:
		server_address = ('127.0.0.1', port)
		myHandlerClass.protocol_version = Protocol
		httpd = HTTPServer(server_address, myHandlerClass)

		sa = httpd.socket.getsockname()
		print "Serving HTTP on", sa[0], "port", sa[1], "..."
		httpd.serve_forever()
	except KeyboardInterrupt:
		print("^C received, shutting down webserver")
		server.socket.close()

if __name__ == '__main__':
	if sys.argv[1:]:
	    port = int(sys.argv[1])
	else:
	    port = 8000
	serve(port)

