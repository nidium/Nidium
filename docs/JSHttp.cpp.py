# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Http", "Download a webpage asynchronously.",
    [ SeeDoc( "Socket" ), SeeDoc( "HTTPServer" ) ],
    [ ExampleDoc( """var h = new Http("http://nidium.com:80/docs");
        function req() {
            h.request({
                headers: {
                    "foo":"bar"
                },
                data: "1234567890",
                timeout: 15,
                maxredirect: 6,
                followlocation: true,
                eval: false,
                path: "docs/api/core/class/Image.html"
            }, function(ev) {
                console.log("Got reply")
                console.log(JSON.stringify(ev));
                req();
            });
        }
        h.onerror = function(err) {
            console.log("Err", JSON.stringify(err))
        }
        req();
""" ) ],
    NO_Inherrits, NO_Extends,
    section="HTTP Client & Server",
    products=["Frontend", "Server"]
)

ConstructorDoc( "Http", "Constructor for a Http Object.",
    NO_Sees,
    NO_Examples,
    [ParamDoc( "url", "The url to download", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "Http instance on success", "Http" )
)

FunctionDoc( "Http.request", "High level api to GET/POST/HEAD a page.",
    [ SeeDoc( "Http.request"), SeeDoc(" Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ), SeeDoc( "Http.onrequest" )  ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "params", "object with details", ObjectDoc([    ("method", "http method type 'POST'|'HEAD'|'GET'|'PUT'|'DELETE'", 'string'), 
                                                                ("headers", "Object with http headers", ObjectDoc([])),
                                                                ("timeout", "Timeout duration for the socket (in ms?)", "integer"), 
                                                                ("maxredirect", "Maximum number of redirects", "integer"), 
                                                                ("data", "The content", "string"), 
                                                                ("eval", "Should the returned data be evaluted as Javascript (?)", "boolean"), 
                                                                ("path", "The requested path", "string"),
                                                                ("Content-Lenght", "the content-length will be set automatically if the 'data' key is set on this object", "integer")]), NO_Default, IS_Obligated ),
      CallbackDoc( "onrequest", "The onrequest event", [ ParamDoc( "response", "Response", "HTTPResponse", NO_Default, IS_Obligated ) ] ) ],
    ReturnDoc( "Http instance", "Http" )
)

EventDoc( "Http.onerror", "Event will be called if an error occurs.",
    [ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
    NO_Examples,
    [ParamDoc( "event", "Event Object", ObjectDoc([("error", "Any of 'http_invalid_response'|'http_server_disconnect'|'http_connection_error'|'http_timedout'|'http_responsecode'", "string")]), NO_Default, IS_Obligated ) ]
)

EventDoc( "Http.data", "Event will be called if an error occurs.",
    [ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
    NO_Examples,
    [ParamDoc( "event", "Event object with keys", ObjectDoc( [("total","Bytes total", "integer"),
                                                          ("read", "Bytes read", "integer"),
                                                          ("type", "Type of data: 'string'|'binary'", "string"),
                                                          ("data", "the content",        "null|string")]),  NO_Default, IS_Obligated ) ]
)

EventDoc( "Http.onrequest", "Handler when a http request is complete.",
    [ SeeDoc( "Http.get" ), SeeDoc( "Http.onerror" ), SeeDoc( "Http.ondata" ) ],
    NO_Examples,
    [ParamDoc( "event", "ventr object with keys", ObjectDoc( [("headers", "A javascript object with the http headers", ObjectDoc([])),
                                                          ("statusCode", "The Http return code", "integer"),
                                                          ("type", "The contenttype: null|'string'|'json'|'binary'", "string"),
                                                          ("data", "the content", "null|string")] ), NO_Default, IS_Obligated) ]
)

