from dokumentor import *

ClassDoc( "Http", "Download a webpage asynchronously.",
	[ SeeDoc( "Socket" ), SeeDoc( "HTTPListener" ) ],
	[ ExampleDoc( """var url = "http://www.somewhere.com/image.png";
var params = { foo : "bar", id : 14 };
var h = new Http(url, params, function(e){
	for (var h in e.headers){
		console.log(h, e.headers[h]);
	}
	console.log("Complete");
});
h.ondata = function(e){
	console.log(e.percent +"%");
};
h.onerror = function(e){
	console.log(e.error);
}; """ ) ],
	NO_Inherrits, NO_Extends
)

ConstructorDoc( "Http", "Constructor for a Http Object.",
	NO_Sees,
	[ ExampleDoc("""
var url = "http://www.somewhere.com/image.png";
var params = { foo : "bar", id : 14 };
var r = new Http(url);
r.ondata = function(e){
    // get the % loaded with e.total and e.read
    var p = Number(e.total) != 0 ? Number(e.read) * 100 / e.total : 0;
    var size = (e.type == "binary" ? e.data.byteLength : e.data.length);
    var percent = Math.round(p*100)/100;
};
r.onerror = function(e){
    console.log('Error: ' + e.error);
};
var options = {
    method : "POST",
    headers : {
        "User-Agent" : "some user agent"
    },
    data : {
        foo : 4,
        bar : 8
    }
};
r.request(options, function(e){
    console.log("Complete.");
    for (var h in e.headers){
        console.log(h + " : " + e.headers[h]);
    }
});""" ) ],
	[ParamDoc( "url", "The url to download", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Http instance on success", "Http" )
)

FunctionDoc( "Http.request", "High level api to GET/POST/HEAD a page.",
	[ SeeDoc( "Http.request"), SeeDoc(" Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ), SeeDoc( "Http.onrequest" )  ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "params", "object with keys: method:'POST'|'HEAD'|'GET'|'PUT'|'DELETE', headers:{}, timeout/integer, maxredirect/integer, data/string, timeout/integer, eval/boolean, path/string. Note: if data is set, then 'Content-Length' will be set in the header", "Object", NO_Default, IS_Obligated ),
	  CallbackDoc( "onrequest", "The onrequest event", [ ParamDoc( "e", "Response", "HTTPResponse", NO_Default, IS_Obligated ) ] ) ],
	ReturnDoc( "Http instance", "Http" )
)

EventDoc( "Http.onerror", "Event will be called if an error occurs.",
	[ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
	NO_Examples,
	[ParamDoc( "e", "Object with keys: error:'http_invalid_response'|'http_server_disconnect'|'http_connection_error'|'http_timedout'|'http_responsecode'", "Object", NO_Default, IS_Obligated ) ]
)

EventDoc( "Http.data", "Event will be called if an error occurs.",
	[ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
	NO_Examples,
	[ParamDoc( "e", "error object with keys: total/integer, read/integer, type:'string'|'binary', data/string", "Object", NO_Default, IS_Obligated ) ]
)

EventDoc( "Http.onrequest", "Handler when a http request is complete.",
	[ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
	NO_Examples,
	[ParamDoc( "e", "error object with keys: headers/object, statusCode/integer, type/null|'string'|'json'|'binary', data/null|string" ) ]
)

