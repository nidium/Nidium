# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("HTTP Client & Server", """Native implemntation of HTTP Client & Server
* HTTP : Connect to a HTTP server.
* HTTPServer : Create an HTTP server where clients cant connect to.
"""
)

ClassDoc( "HTTPServer", "Http server.",
    [ SeeDoc( "HTTPRequest" ), SeeDoc( "Socket" ), SeeDoc( "Http" ) ],
    [ ExampleDoc( """var ws = new HTTPServer(  8080, true, "127.0.0.1" );
ws.onrequest = function( request, client ) {
    console.log( request.method + " " + request.url );
    console.log( JSON.stringify( request.headers ) );
    console.log( JSON.stringify( client ) );
    client.write( "hello, this is nidium" );
} ;""") ],
    NO_Inherrits,
    NO_Extends,
        section="HTTP Client & Server",
)

ClassDoc( "HTTPRequest", "Http request object spawned by HTTPServer.",
    [ SeeDoc( "HTTPServer" ), SeeDoc( "Socket" ), SeeDoc( "Http" ) ],
    NO_Examples,
    NO_Inherrits,
    NO_Extends,
        section="HTTP Client & Server",
)

NamespaceDoc( "HTTPResponse", "HTTPResponse class.",
    SeesDocs( "HTTPServer|HTTPRequest|Http" ),
    NO_Examples,
        section="HTTP Client & Server",
)

EventDoc( "HTTPServer.onDisconnect", "Event that fires on disconnect.",
    [ SeeDoc( "HTTPServer.ondisconnect" ), SeeDoc( "HTTPServer.ondata" ), SeeDoc( "HTTPServer.onrequest" ) ],
    NO_Examples,
    NO_Params
)

EventDoc( "HTTPServer.ondata", "Event that fires on data.",
    [ SeeDoc( "HTTPServer.ondisconnect" ), SeeDoc( "HTTPServer.ondata" ), SeeDoc( "HTTPServer.onrequest" ) ],
    NO_Examples,
    NO_Params
)

FieldDoc( "HTTPRequest.method", "Http request Method that was recieved. Can be 'POST'|'GET'|'PUT'|'DELETE'|'UNKNOWN'.",
    [ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
     "UNKNOWN"
)

FieldDoc( "HTTPRequest.url", "The url that was requested.",
    [ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
     NO_Default
)

FieldDoc( "HTTPRequest.data", "The data (utf-8') that was received by the server.",
    [ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
     'null, if it was a non POST method'
)

FieldDoc( "HTTPRequest.headers", "An object of key/value pairs describing the headers.",
    [ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    ObjectDoc([]),
     NO_Default
)

FieldDoc( "HTTPRequest.client", "An object that describes the connected client.",
    [ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    ObjectDoc([]),
     NO_Default
)

EventDoc( "HTTPServer.onrequest", "Event that fires when the server has read the complete http request.",
    [ SeeDoc( "HTTPServer.ondisconnect" ), SeeDoc( "HTTPServer.ondata" ), SeeDoc( "HTTPServer.onrequest" ) ],
    NO_Examples,
    [   ParamDoc( "request", "client request", "HTTPRequest", NO_Default, IS_Obligated ),
        ParamDoc( "response", "Client response", "HTTPResponse", NO_Default, IS_Obligated ) ]
)

ConstructorDoc( "HTTPServer", "Constructor for HTTPServer object.",
    NO_Sees,
    NO_Examples,
    [ 
        ParamDoc( "port", "The port to listen to", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "ip", "The ip address to bind to", "string", "0.0.0.0", IS_Optional ), 
        ParamDoc("options", "HTTPListner options",
            ObjectDoc([
                ("reusePort", "Allows multiple HTTPServer to bind to the same port,", "boolean", 'false'),
        ]), NO_Default, IS_Optional)
    ],
    ReturnDoc( "Webserver object", "HTTPServer" )
)

FunctionDoc( "HTTPRequest.write", "Respond to a client that made a request to the webserver.",
    [ SeeDoc( "HTTPServer" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "data", "The data to send out", "string|ArrayBuffer", IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "HTTPRequest.end", """End a responce to a client that made a request to the webserver

If the headers were not send yet, a HTTP code 200 will be send.""",
    [ SeeDoc( "HTTPServer" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "data", "The data to send out", "string|ArrayBuffer", IS_Optional ) ],
    NO_Returns
)

FunctionDoc( "HTTPRequest.writeHead", "Set the headers on a response to a client that made a request to the webserver. Of course this can only happen if the headers were not send before.",
    [ SeeDoc( "HTTPServer" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "statuscode", "The HTTP code to set (e.g. 200)", "integer", IS_Obligated ),
      ParamDoc( "kvpairs", "Header data object", ObjectDoc([]), IS_Optional ) ],
    NO_Returns
)

