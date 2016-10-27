#!/usr/bin/env python
# -*- coding: utf-8 -*-

# WebSocket echo echo server with SSL support
# Modified version of https://github.com/Pithikos/python-websocket-server
# Author: Johan Hanssen Seferidis
# License: MIT

import re, sys
import os
import struct
from base64 import b64encode
from hashlib import sha1
import logging
import ssl

if sys.version_info[0] < 3 :
    from SocketServer import ThreadingMixIn, TCPServer, StreamRequestHandler
else:
    from socketserver import ThreadingMixIn, TCPServer, StreamRequestHandler

'''
+-+-+-+-+-------+-+-------------+-------------------------------+
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
'''

FIN    = 0x80
OPCODE = 0x0f
MASKED = 0x80
PAYLOAD_LEN = 0x7f
PAYLOAD_LEN_EXT16 = 0x7e
PAYLOAD_LEN_EXT64 = 0x7f

OPCODE_TEXT = 0x01
OPCODE_BINARY = 0x02
CLOSE_CONN  = 0x8


# ------------------------------ Logging -------------------------------
logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger("WebSocket")

# -------------------------------- API ---------------------------------

class API():
    def run_forever(self):
        try:
            logger.info("Listening on port %d%s for clients.." % (self.port, " (SSL enabled)" if self.useSSL else ""))
            self.serve_forever()
        except KeyboardInterrupt:
            self.server_close()
            logger.info("Server terminated.")
        except Exception as e:
            logger.error("ERROR: WebSocketsServer: " + str(e), exc_info=True)
            exit(1)

    def serve_forever (self):
        self.stop = False
        while not self.stop:
            self.handle_request()

    def new_client(self, client, server):
        logger.info("New client")
    def client_left(self, client, server):
        logger.info("Client left")
    def message_received(self, client, server, message):
        logger.info("Received : %s" % message)
        self.send_message(client, message);

    def send_message(self, client, msg):
        self._unicast_(client, msg)
    def send_message_to_all(self, msg):
        self._multicast_(msg)



# ------------------------- Implementation -----------------------------

class WebsocketServer(ThreadingMixIn, TCPServer, API):

    allow_reuse_address = True
    daemon_threads = True # comment to keep threads alive until finished

    '''
    clients is a list of dict:
        {
         'id'      : id,
         'handler' : handler,
         'address' : (addr, port)
        }
    '''
    clients=[]
    id_counter=0

    def __init__(self, port, host='127.0.0.1', useSSL=False):
        self.port = port
        self.useSSL = useSSL
        TCPServer.__init__(self, (host, port), WebSocketHandler)

    def get_request(self):
        (socket, addr) = TCPServer.get_request(self)

        newsock = None

        if self.useSSL:
            try:
                path = os.path.dirname(os.path.realpath(__file__))
                newsock = ssl.wrap_socket(socket,
                        server_side=True,
                        # On my system, only AES256 works, so keep it around as a fallback
                        ciphers="HIGH:AES256",
                        certfile=os.path.join(path, "localhost.cert"),
                        keyfile=os.path.join(path, "localhost.key"))
            except Exception as e:
                import traceback
                traceback.print_exc()
        else:
            newsock = socket

        return newsock, addr

    def _message_received_(self, handler, msg):
        self.message_received(self.handler_to_client(handler), self, msg)

    def _new_client_(self, handler):
        self.id_counter += 1
        client={
            'id'      : self.id_counter,
            'handler' : handler,
            'address' : handler.client_address
        }
        self.clients.append(client)
        self.new_client(client, self)

    def _client_left_(self, handler):
        client=self.handler_to_client(handler)
        self.client_left(client, self)
        if client in self.clients:
            self.clients.remove(client)

    def _unicast_(self, to_client, msg):
        to_client['handler'].send_message(msg)

    def _multicast_(self, msg):
        for client in self.clients:
            self._unicast_(client, msg)

    def handler_to_client(self, handler):
        for client in self.clients:
            if client['handler'] == handler:
                return client



class WebSocketHandler(StreamRequestHandler):

    def __init__(self, socket, addr, server):
        self.server=server
        StreamRequestHandler.__init__(self, socket, addr, server)

    def setup(self):
        StreamRequestHandler.setup(self)
        self.keep_alive = True
        self.handshake_done = False
        self.header = ""
        self.valid_client = False

    def handle(self):
        while self.keep_alive:
            if not self.handshake_done:
                self.handshake()
            elif self.valid_client:
                self.read_next_message()

    def read_bytes(self, num):
        # python3 gives ordinal of byte directly
        bytes = self.rfile.read(num)
        if sys.version_info[0] < 3:
            return map(ord, bytes)
        else:
            return bytes

    def read_next_message(self):
        try:
            b1, b2 = self.read_bytes(2)
        except ValueError as e:
            b1, b2 = 0, 0

        fin    = b1 & FIN
        opcode = b1 & OPCODE
        masked = b2 & MASKED
        payload_length = b2 & PAYLOAD_LEN

        if not b1:
            logger.info("Client closed connection.")
            self.keep_alive = 0
            return
        if opcode == CLOSE_CONN:
            logger.info("Client asked to close connection.")
            self.keep_alive = 0
            return
        if not masked:
            logger.info("Client must always be masked.")
            self.keep_alive = 0
            return

        if payload_length == 126:
            payload_length = struct.unpack(">H", self.rfile.read(2))[0]
        elif payload_length == 127:
            payload_length = struct.unpack(">Q", self.rfile.read(8))[0]

        if opcode == OPCODE_BINARY:
            decoded = bytearray()
        else:
            decoded = ""

        masks = self.read_bytes(4)
        for char in self.read_bytes(payload_length):
            char ^= masks[len(decoded) % 4]
            if opcode == OPCODE_BINARY:
                decoded.append(char)
            else:
                decoded += chr(char)
        self.server._message_received_(self, decoded)

    def send_message(self, message):
        self.send_text(message)

    def send_text(self, message):
        '''
        NOTES
        Fragmented(=continuation) messages are not being used since their usage
        is needed in very limited cases - when we don't know the payload length.
        '''

        opcode = OPCODE_TEXT
        if isinstance(message, bytearray):
            opcode = OPCODE_BINARY
            payload = message
        elif isinstance(message, bytes):
            message = try_decode_UTF8(message) # this is slower but assures we have UTF-8
            if not message:
                logger.warning("Can\'t send message, message is not valid UTF-8")
                return False
            payload = encode_to_UTF8(message)
        elif isinstance(message, str) or isinstance(message, unicode):
            payload = encode_to_UTF8(message)
        else:
            logger.warning('Can\'t send message, message has to be a bytearray, string or bytes. Given type is %s' % type(message))
            return False

        header  = bytearray()
        payload_length = len(payload)

        # Normal payload
        if payload_length <= 125:
            header.append(FIN | opcode)
            header.append(payload_length)

        # Extended payload
        elif payload_length >= 126 and payload_length <= 65535:
            header.append(FIN | opcode)
            header.append(PAYLOAD_LEN_EXT16)
            header.extend(struct.pack(">H", payload_length))

        # Huge extended payload
        elif payload_length < 18446744073709551616:
            header.append(FIN | opcode)
            header.append(PAYLOAD_LEN_EXT64)
            header.extend(struct.pack(">Q", payload_length))

        else:
            raise Exception("Message is too big. Consider breaking it into chunks.")
            return

        self.request.send(header + payload)

    def handshake(self):
        logger.info("Reading HTTP request header")
        tmp = self.request.recv(1024)
        if not tmp:
            self.keep_alive = False
            return

        self.header += tmp
        if self.header.find('\r\n\r\n') == -1:
            logger.warning("Haven't received the full HTTP headers. Will try again.")
            return

        self.header = self.header.strip()

        if self.header.find("GET /kill") != -1:
            logger.info("Killing server")
            self.request.send("HTTP/1.1 200 OK\r\n\r\n")
            self.keep_alive = False
            self.server.shutdown()
            return

        upgrade = re.search('\nupgrade[\s]*:[\s]*websocket', self.header.lower())
        if not upgrade:
            self.request.send("HTTP/1.1 200 OK\r\n\r\n")
            self.keep_alive = False
            return

        key = re.search('[sS]ec-[wW]eb[sS]ocket-[kK]ey[\s]*:[\s]*(.*)', self.header)
        if key:
            key = key.group(1)
        else:
            logger.warning("Client tried to connect but was missing a key")
            self.keep_alive = False
            return

        logger.info("Doing WebSocket handshake")

        response = self.make_handshake_response(key)
        self.handshake_done = self.request.send(response.encode())
        self.valid_client = True
        self.server._new_client_(self)

    def make_handshake_response(self, key):
        return \
          'HTTP/1.1 101 Switching Protocols\r\n'\
          'Upgrade: websocket\r\n'              \
          'Connection: Upgrade\r\n'             \
          'Sec-WebSocket-Accept: %s\r\n'        \
          '\r\n' % self.calculate_response_key(key)

    def calculate_response_key(self, key):
        GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
        hash = sha1(key.encode() + GUID.encode())
        response_key = b64encode(hash.digest()).strip()
        return response_key.decode('ASCII')

    def finish(self):
        self.server._client_left_(self)



def encode_to_UTF8(data):
    try:
        return data.encode('UTF-8')
    except UnicodeEncodeError as e:
        logger.error("Could not encode data to UTF-8 -- %s" % e)
        return False
    except Exception as e:
        raise(e)
        return False



def try_decode_UTF8(data):
    try:
        return data.decode('utf-8')
    except UnicodeDecodeError:
        return False
    except Exception as e:
        raise(e)



# This is only for testing purposes
class DummyWebsocketHandler(WebSocketHandler):
    def __init__(self, *_):
        pass

def serve(port, bind, useSSL):
    server = WebsocketServer(port, bind, useSSL=useSSL)
    server.run_forever()

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Create an echo websocket server')
    parser.add_argument('--bind', action="store", dest="bind", type=str, default="127.0.0.1")
    parser.add_argument('--port', action="store", dest="port", type=int, default=9000)
    parser.add_argument('--ssl', action="store_true", dest="ssl", default=False)
    parser.add_argument('--fork', action="store_true", dest="fork", default=False)

    logging.basicConfig(level=logging.DEBUG, format="%(asctime)s - %(levelname)s - %(message)s")

    args = parser.parse_args()

    if args.fork:
        pid = os.fork();
        if pid == 0:
            serve(args.port, args.bind, args.ssl)
    else:
        serve(args.port, args.bind, args.ssl)
