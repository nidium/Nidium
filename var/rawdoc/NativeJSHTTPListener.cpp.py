from dokumentor import *

ClassDoc( "HTTPListener", "Http server",
	[ SeeDoc( "HTTPRequest" ), SeeDoc( "Socket" ), SeeDoc( "Http" ) ],
	[ ExampleDoc( """var ws = new HTTPListener(  8080, true, "127.0.0.1" );
ws.onrequest = function( request, client ) {
	console.log( request.method + " " + request.url );
	console.log( JSON.stringify( request.headers ) );
	console.log( JSON.stringify( client ) );
	client.write( "hello, this is nidium" );
} ;""") ],
	NO_Inherrits,
	NO_Extends
)

ClassDoc( "HTTPRequest", "Http request object spawned by HTTPListener",
	[ SeeDoc( "HTTPListener" ), SeeDoc( "Socket" ), SeeDoc( "Http" ) ],
	NO_Examples,
	NO_Inherrits,
	NO_Extends
)

NamespaceDoc( "HTTPResponse", "HTTPResponse class",
	SeesDocs( "HTTPListener|HTTPRequest|Http" ),
	NO_Examples
)

EventDoc( "HTTPListener.onDisconnect", "Event that fires on disconnect",
	[ SeeDoc( "HTTPListener.ondisconnect" ), SeeDoc( "HTTPListener.ondata" ), SeeDoc( "HTTPListener.onrequest" ) ],
	NO_Examples,
	NO_Params
	)

EventDoc( "HTTPListener.ondata", "Event that fires on data",
	[ SeeDoc( "HTTPListener.ondisconnect" ), SeeDoc( "HTTPListener.ondata" ), SeeDoc( "HTTPListener.onrequest" ) ],
	NO_Examples,
	NO_Params
	)

FieldDoc( "HTTPRequest.method", "Http request Method that was recieved. Can be 'POST', 'GET', 'PUT', 'DELETE' or 'UNKNOWN'",
	[ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'string',
	 "UNKNOWN"
	)
FieldDoc( "HTTPRequest.url", "The url that was requested",
	[ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'string',
	 NO_Default
	)
FieldDoc( "HTTPRequest.data", "The data (utf-8') that was url that was received by the server",
	[ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'string',
	 'null, if it was a non POST method'
	)
FieldDoc( "HTTPRequest.headers", "An object of key value pairs",
	[ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'object',
	 NO_Default
	)
FieldDoc( "HTTPRequest.client", "An object that describes the connected client",
	[ SeeDoc( "HTTPRequest.data" ), SeeDoc( "HTTPRequest.method" ), SeeDoc( "HTTPRequest.headers" ), SeeDoc( "HTTPRequest.client" ), SeeDoc( "HTTPRequest.url" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'object',
	 NO_Default
	)
EventDoc( "HTTPListener.onrequest", "Event that fires when the server has read the complete http request",
	[ SeeDoc( "HTTPListener.ondisconnect" ), SeeDoc( "HTTPListener.ondata" ), SeeDoc( "HTTPListener.onrequest" ) ],
	NO_Examples,
	[ ParamDoc( "request", "client request object", "HTTPRequest", NO_Default, IS_Obligated ),
      ParamDoc( "response", "Client response object", "HTTPResponse", NO_Default, IS_Obligated ) ]
	)

ConstructorDoc( "HTTPListener", "Constructor for HTTPListener object",
	NO_Sees,
	NO_Examples,
	[ ParamDoc( "port", "The port to listen to", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "reuse", "Re-use address/port", "boolean", 'true', IS_Optional ),
	  ParamDoc( "ip", "The ip address to bind to", "string", "0.0.0.0", IS_Optional ), ],
	ReturnDoc( "Webserver object", "HTTPListener" )
)

FunctionDoc( "HTTPRequest.write", "Respond to a client that made a request to the webserver",
	[ SeeDoc( "HTTPListener" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "data", "The data to send out", "string|ArrayBuffer", IS_Obligated ) ],
	NO_Returns
);

FunctionDoc( "HTTPRequest.end", "End a responce to a client that made a request to the webserver",
	[ SeeDoc( "HTTPListener" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "data", "The data to send out", "string|ArrayBuffer", IS_Optional ) ],
	NO_Returns
 );

FunctionDoc( "HTTPRequest.writeHead", "Set the headers on a responce to a client that made a request to the webserver. Of course this can only happen if the headers were not send before.",
	[ SeeDoc( "HTTPListener" ), SeeDoc( "HTTPRequest.write"), SeeDoc( "HTTPRequest.end" ), SeeDoc( "HTTPRequest.writeHead" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "statuscode", "The HTTP code to set (e.g. 200)", "integer", IS_Obligated ),
	  ParamDoc( "kvpairs", "Header data object", "object", IS_Optional ) ],
	NO_Returns
  );
