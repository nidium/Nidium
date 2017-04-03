# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Socket", """An advanced non-blocking socket class.

Nidium Socket API was designed to connect to:

*  any host:port as a client
* to listen to a port as a server to handle incomming client connections.

The underlying system is powered by a powerful, non-blocking event-driven engine.""",
    SeesDocs( "Http|HTTPServer|SocketClient|WebSocket|WebSocketClient" ),
    [ ExampleDoc( """//Client Example
var socket = new Socket("nidium.com", 80).connect();
socket.onconnect = function() {
    this.write("GET / HTTP/1.1\\n");
    this.write("Host: www.nidium.com\\n\\n");
}
socket.ondisconnect = function() {
    console.log("Disconnected");
}
socket.onread = function(data) {
    //console.log("=>", data);
}"""), ExampleDoc( """//Server example
var socket = new Socket("0.0.0.0", 8000).listen();
socket.onaccept = function(clientSocket) {
    clientSocket.write("Hello");
}
socket.ondisconnect = function(clientSocket) {
    console.log("Client Disconnected");
}
socket.onread = function(clientSocket, data) {
    //console.log("=>", data);
}""") ],
    NO_Inherrits,
    NO_Extends,
    products=["Frontend", "Server"]
)

ClassDoc( "SocketClient", "A connected client.",
    SeesDocs( "Http|HTTPServer|SocketClient|WebSocket|WebSocketClient" ),
    NO_Examples,
    [ "Socket" ],
    NO_Extends,
        section="Socket"
)

FieldDoc( "Socket.binary", """Get or set the binary mode for send/receive data.

When the 'Socket.binary' property is set to 'true', the 'Socket.ondata' event will be called with an 'ArrayBuffer' instead of a 'string'.""",
    SeesDocs( "Socket.encoding|Socket.timeout|Socket.readline|Socket.binary|Socket" ),
     [ ExampleDoc( """var socket = new Socket("0.0.0.0", 8001).listen()
socket.binary = true;
socket.onread = function(clientSocket, data) {
    console.log("=>", data); // data is an ArrayBuffer
}""") ],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    'boolean',
    NO_Default
)

FieldDoc( "Socket.timeout", """Get or set the timeout on a socket connection.""",
    #@TODO: what is 0? what is the unit? ms/sec
    SeesDocs( "Socket.encoding|Socket.timeout|Socket.readline|Socket.binary|Socket" ),
     NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    'integer',
    NO_Default
)

FieldDoc( "Socket.readline", """Handle 'Socket.onread' events on a one line-per-line basis. A TCP Socket stream is 'frameless'.

A TCP Socket stream is 'frameless'. When you receive data, there is no concept of boundaries.
For instance, if an endpoint sends "hello" it doesn't mean you can't receive the data in 2 or more callbacks.
The 'readline' property tells the socket to set boundaries around the newline char (\\n).
This is particularly useful to design simple protocols like IRC which are line-based.

The maximum length of a line is internally set to 8192 bytes.""",
    SeesDocs( "Socket.encoding|Socket.onread|Socket.readline|Socket.binary|Socket" ),
    [ExampleDoc("""var socket = new Socket("irc.freenode.org", 6667).connect();
socket.readline = true;
socket.onconnect = function(clientSocket) {
    clientSocket.write("NICKNAME foo\\n");
    clientSocket.write("USER A A A A\\n");
}
// if the server sends something like :
// HELLO\\nPING 123\\nFOOBAR, the "onread" callback is called twice (once with HELLO and once with PING).
// The remaining data (FOOBAR) is held in memory until a "\\n" is encountered.
socket.onread = function(data) {
    console.log("=>", data);
}
""") ],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    'boolean',
    NO_Default
)
FieldDoc( "Socket.encoding", """The encoding to read non-binary data.

When the 'Socket.binary' property is set to 'false' (its default value), the 'Socket.encoding' property defines the encoding of received data.
Allowed values are 'undefined'|'ASCII'|'utf8'.""",
    SeesDocs( "Socket.encoding|Socket.readline|Socket.binary|Socket" ),
    [ExampleDoc("""var socket = new Socket("127.0.0.1", 6667).connect();
socket.binary = false;
socket.encoding = 'utf8';
// if the server sends something like :
socket.onread = function(data) {
    console.log("=>", data);
} """) ],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    'string',
    'ascii'
)

EventDoc( "Socket.onconnect", """Function to execute on a socket if client called connect and the connection has been established correctly.

This method is called when a new client connects on a listening Socket.""",
    SeesDocs( "Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    [ ExampleDoc( """var socket = new Socket("0.0.0.0", 8002).listen();
socket.onaccept = function(clientSocket) {
    clientSocket.write("hello !\\n");
}""" ), ExampleDoc("""var socket = new Socket("irc.freenode.org", 6667).connect();
socket.onconnect = function( clientSocket ) {
    clientSocket.write("NICKNAME foo\\n");
    clientSocket.write("USER A A A A\\n");
}""") ],
    NO_Params
)

FieldDoc( "SocketClient.ip", "The ip address of the connected Socket.",
    SeesDocs( "Socket|HTTPServer" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
    NO_Default,
)
EventDoc( "Socket.onaccept", """Function to execute on a server when a client connects.""",
    SeesDocs( "Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    [ExampleDoc("""var socket = new Socket("0.0.0.0", 8003).listen();
socket.onaccept = function(clientSocket) {
    clientSocket.write("hello !\\n");
}""")],
    [ParamDoc( "clientSocket", "Socketclient instance", "SocketClient", NO_Default, IS_Obligated ) ]
)

EventDoc( "Socket.onread", "Function to execute on a socket upon the read event.",
    SeesDocs( "Socket.binary|Socket.readline|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    NO_Examples,
    [ ParamDoc( "data", "The message", "string", NO_Default, IS_Obligated ) ],
)

EventDoc( "Socket.ondrain", """Function to execute on a socket if it has been drained.

That is: after a 'Socket.write' call, when everything was written, and the socket is ready for writing again.""",
    SeesDocs( "Socket.binary|Socket.readline|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    NO_Examples,
    NO_Params
)

EventDoc( "Socket.onmessage", "Function to execute when a message is available on a 'udp' socket.",
    SeesDocs( "Socket.binary|Socket.readline|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    [ ExampleDoc("""var socket = new Socket("0.0.0.0", 8004).listen("udp");
socket.onmessage = function(data, details) {
    console.log("A message has arrived from", details.ip, ":", details.port);
    console.log("=>", data);
}""") ],
    [ ParamDoc( "data", "The message", "string", NO_Default, IS_Obligated ),
     ParamDoc( "details", "Connection details object", ObjectDoc([("ip", "The remote IP address", "string"), ("port", "The connected port", "integer")]), NO_Default, IS_Obligated ) ],
)

EventDoc( "SocketClient.onread", "Function to execute on a 'SocketClient', if it is not binary and not-readline.",
    SeesDocs( "Socket.binary|Socket.readline|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    NO_Examples,
    [ ParamDoc( "socketClient", "The connected client", "SocketClient", NO_Default, IS_Obligated ),
      ParamDoc( "data", "The read data", "string", NO_Default, IS_Obligated ) ]
)

EventDoc( "SocketClient.ondisconnect", "Function to execute on a 'SocketClient' upon the disconnect event.",
    SeesDocs( "Socket.binary|Socket.readline|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    [ ExampleDoc("""var socket = new Socket("irc.freenode.org", 6667).connect();
socket.onconnect = function(clientSocket) {
    clientSocket.write("NICKNAME foo\\n");
    clientSocket.disconnect( );
}
socket.ondisconnect=  function( clientSocket ) {
    console.log( "Disconnected from: " + clientSocket.ip );
}""") ],
    [ParamDoc( "clientSocket", "Socketclient instance", "SocketClient", NO_Default, IS_Obligated ) ]
)

EventDoc( "Socket.ondisconnect", "Function to execute on a socket upon the disconnect event.",
    SeesDocs( "SocketClient.ondisconnect|Socket.ondisconnect|Socket.onconnect|Socket.onaccept|Socket.onread|Socket.ondrain|Socket.onmessage|Socket.onread|Socket.ondisconnect" ),
    NO_Examples,
    [ParamDoc( "client", "Socketclient instance", "SocketClient", NO_Default, IS_Obligated ) ]
)

ConstructorDoc( "Socket", "Create a new Socket object ready for connecting or listening.",
    SeesDocs( "Http|HTTPServer|Socket.encoding|Socket.readline|Socket.binary|Socket|SocketClient" ),
    [ ExampleDoc("""//Client
var client = new Socket("google.com", 80);
client.connect();
""" ), ExampleDoc("""//Server
var server = new Socket("0.0.0.0", 8005);
server.listen();""" ) ],
    [ ParamDoc( "host", "Hostname or ip-address. '0.0.0.0' as host means any host when used with 'listen()' (e.g. when the socket must be accessed from the Internet). If an name is given, the name is resolved asynchronously automatic.", "string", NO_Default, IS_Obligated ),
     ParamDoc( "port", "port number", 'integer', NO_Default, IS_Obligated ) ],
    ReturnDoc( "a (non-binary) socket instance", "Socket" )
)

FunctionDoc( "Socket.listen", """Starts a server.

The server will listen to the host and port that were specified in the constructor.""",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.disconnect|HTTPServer" ),
    [ExampleDoc("""var socket = new Socket("0.0.0.0", 8006).listen();

socket.onaccept = function(clientSocket) {
    clientSocket.write("hello !\\n");
    clientSocket.disconnect();
}""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "mode", "Connection mode. Allowed values are: 'tcp'|'udp'|'ssl'|'unix'|'tcp-lz4'", "string", 'tcp', IS_Optional ) ],
    ReturnDoc( "The socket, for chaining", "Socket" )
)

FunctionDoc( "Socket.connect", """Starts a client.

Connect the socket to the tuple host and port that were specified in the constructor.""",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.disconnect|HTTPServer" ),
     [ExampleDoc("""var client = new Socket("nidium.com", 80).connect();"""),
    ExampleDoc("""var client = new Socket("google.com", 80).connect("ssl");""") ] ,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "mode", "Conncection mode. Allowed values are: 'tcp'|'udp'|'ssl'|'unix'|'tcp-lz4'", "string", 'tcp', IS_Optional ) ],
    ReturnDoc( "The socket, for chaining", "Socket" )
)

FunctionDoc( "Socket.sendFile", """Send a file to a client.

Send a file via the 'sendfile' system call. This is not available on windows.""",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.disconnect|HTTPServer" ),
     [ExampleDoc("""var client = new Socket("nidium.com", 80).connect();
socket.onconnect = function(clientSocket) {
    clientSocket.sendFile("document.docx");
    clientSocket.disconnect()
}""", runCode=False) ],
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( "mode", "Conncection mode. Allowed values are: 'tcp'|'udp'|'ssl'|'unix'|'tcp-lz4'", "string", 'tcp', IS_Optional ) ],
    ReturnDoc( "The socket, for chaining", "Socket" )
)


FunctionDoc( "SocketClient.write", """Write data to a connected server/client socket.

If 'Socket.write' returns 0, the data is internally saved and sent as soon as possible.
In order to avoid high memory usage, one should stop calling 'Socket.write' when a previous call to 'Socket.write' returned '0' and wait for the 'Socket.ondrain' event to be sent.
""",
    SeesDocs( "Socket.listen|SocketClient.sendFile|SocketClient.write|SocketClient.disconnect|HTTPServer" ).append( "Socket.write " ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( 'data', "data to send", "string|ArrayBuffer", NO_Default, IS_Obligated ) ],
    ReturnDoc( "bytes written", "integer" )
)

FunctionDoc( "Socket.write", "Write data.",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.write|SocketClient.disconnect|HTTPServer" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc( 'data', "data to send", "string|ArrayBuffer", NO_Default, IS_Obligated ) ],
    ReturnDoc( "bytes written", "integer" )
)

FunctionDoc( "SocketClient.disconnect", """Close the socket on an existing connection.

This can be done by both the server (after the 'Socket.onaccept') and the client (after the 'Socket.connect').""",
    SeesDocs( "SocketClient.sendFile|SocketClient.write|SocketClient.disconnect|HTTPServer|Socket.connect|Socket.listen" ),
    [ExampleDoc("""var s = new Socket("0.0.0.0", 8007).listen();

s.onaccept = function(clientSocket) {
    clientSocket.write("hello !\\n");
    clientSocket.disconnect();
}"""), ExampleDoc("""var socket = new Socket("irc.freenode.org", 6667).connect();

socket.onconnect = function(clientSocket) {
    clientSocket.write("NICKNAME foo\\n");
    clientSocket.write("USER A A A A\\n");
    clientSocket.disconnect()
}""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Socket.close", "Close a socket connection.",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.write|SocketClient.disconnect|HTTPServer" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Socket.sendTo", "Send data from a socket server to a client via udp.",
    SeesDocs( "Socket.listen|Socket.connect|Socket.write|Socket.disconnect|Socket.SendTo|SocketClient.write|SocketClient.disconnect|HTTPServer" ),
    [ExampleDoc("""var s = new Socket("0.0.0.0", 8008).listen("udp");

s.onaccept = function(clientSocket) {
    clientSocket.sendTo( clientSocket.ip, clientSocket.port, "hello !\\n");
    clientSocket.disconnect();
}""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "ip", "The Ip to send to", 'string', NO_Default, IS_Obligated ),
        ParamDoc( "port", "Portnumber", 'integer', NO_Default, IS_Obligated ),
        ParamDoc( "data", "The data to send", "string|ArrayBuffer", NO_Default, IS_Obligated )
    ],
    NO_Returns
)

