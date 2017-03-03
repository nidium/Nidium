# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

optionParam = ParamDoc("params", "object with details", ObjectDoc([
    ("method", "http method type `POST`, `HEAD`, `GET`, `PUT`, `DELETE`", 'string'),
    ("headers", "Object with http headers", ObjectDoc([])),
    ("timeout", "Timeout duration for the socket (in milliseconds)", "integer"),
    ("maxRedirects", "Maximum number of redirects", "integer"),
    ("data", "The content (POST method only).", "string|ArrayBuffer"),
    ("eval", "Evaluate the data based on the `content-type` header for now only `application/json` and `text/html` are supported, other content-type are converted to an `ArrayBuffer`, `", "boolean", True),
    ("path", "The requested path", "string"),
    ("followLocation", "Follow HTTP redirect", "boolean", False),
]), NO_Default, IS_Optional)

responseEventObject = [
    ("headers", "An object representing the response headers", ObjectDoc([])),
    ("statusCode", "HTTP status code", "integer", "integer"),
    ("data", "Response data", "void|object|string|ArrayBuffer|Image"),
    ("type", "The type of the data returned : `String`, `json`, `binary`", "string")
]

# {{{ Constructor, Methods
ClassDoc("HTTP", "Fetch a resource over HTTP",
    [SeeDoc("Socket" ), SeeDoc( "HTTPServer")],
    [
        ExampleDoc( """new HTTP("http://www.nidium.com/", function(ev) {
    console.log(ev.data);
});""", title="Short hand mode"),
        ExampleDoc( """var h = new HTTP("http://www.nidium.com/");

h.addEventListener("error", function(ev) {
    console.log("Error " + ev.errorCode + " : " + ev.error);
});

h.addEventListener("response", function(ev) {
    console.log("Received headers", JSON.stringify(ev.headers));
    console.log("Status code is ", ev.statusCode);
    console.log("Received data ", ev.data);
});

h.request({
    method: "GET",
    headers: {"foo": "bar"}
});""", title="Full example")
    ],
    NO_Inherrits, NO_Extends,
    section="HTTP Client & Server",
    products=["Frontend", "Server"]
)

ConstructorDoc( "HTTP", """HTTP Object constructor.

If `params` and  `callback` arguments are providen the HTTP request will be executed right away. Otherwise, you'll have to call the `request` method and listen for the `response` event.""",
    params=[
        ParamDoc("url", "The url to fetch", "string", NO_Default, IS_Obligated ),
        optionParam,
        CallbackDoc("callback", "Function called on error or when the request is finished", [
            ParamDoc("event", "Event Object or Error Event Object. See `error` and `response` event. ", ObjectDoc([]))
        ], NO_Default, IS_Optional)
    ],
    returns=ReturnDoc( "HTTP instance on success", "HTTP" )
)

FunctionDoc( "HTTP.request", """Perform the HTTP request

Once a request is finished (`response` or `error` event has been fired), you can call the `request` method once again to run a new HTTP request. The options from the first request wil be reset.""",
    SeesDocs("HTTP.error|HTTP.progress|HTTP.response|HTTP.stop"),
    [ExampleDoc( """var h = new HTTP("http://www.nidium.com/");

h.addEventListener("error", function(ev) {
    console.log("Error " + ev.errorCode + " : " + ev.error);
});

h.addEventListener("response", function(ev) {
    console.log("Received headers", JSON.stringify(ev.headers));
    console.log("Status code is ", ev.statusCode);
    console.log("Received data ", ev.data);
});

h.request({
    method: "GET",
    headers: {"foo": "bar"}
});""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [optionParam],
    ReturnDoc( "HTTP instance", "HTTP" )
)

FunctionDoc("HTTP.stop", "Stop a pending request.\n\n If a request is pending the `error` event will be fired.",
    SeesDocs("HTTP.request"),
    [ExampleDoc("""var h = new HTTP("http://www.nidium.com");

h.addEventListener("error", function(ev) {
    console.log("Got an error", ev.error);
});

h.request();

setImmediate(function() {
    h.stop();
});
""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "HTTP instance", "HTTP" )
)
# }}}

# {{{ Events
EventDoc("HTTP.error", "Event called if an error occurs.",
    SeesDocs("HTTP.response|HTTP.progress"),
    NO_Examples,
    [
        ParamDoc("event", "Error Event Object", ObjectDoc([
            ("error", "The error string.", "string"),
            ("errorCode", "The error code.", "integer"),
        ]), NO_Default, IS_Obligated )
    ]
)

EventDoc("HTTP.response", """Event called when all the data has been read.

The received headers are analyzed and the value of 'event.data' and 'event.type' can be changed:
* mimetype='application/octet-stream': event.type='binary' and event.data is set to an ArrayBuffer object.
* mimetype='application/json': and valid data: event.type='json' and event.data is the parsed javascript value.
* mimetype='application/json': and in valid data: event.type='string' and event.data is the original string.
* mimetype='image/jpg' or 'image/png': event.type='image' and event.data is set to an Image object. (Frontend only)
""",
	#todo audio
    SeesDocs("HTTP.error|HTTP.progress|HTTP.response"),
    NO_Examples,
    [ParamDoc("event", "Event object with keys", ObjectDoc(responseEventObject),  NO_Default, IS_Obligated )]
)

EventDoc("HTTP.headers", "Event called when all the headers has been received.",
    SeesDocs("HTTP.error|HTTP.progress"),
    [ExampleDoc("""var h = new HTTP("http://www.nidium.com");
h.addEventListener("headers", function(ev) {
    console.log("Received headers", JSON.stringify(ev.headers));
    console.log("Status code is ", ev.statusCode);
});

h.request();""")],
    params = [ParamDoc("event", "Event object with keys", ObjectDoc([
        ("headers", "An object representing the response headers", ObjectDoc([])),
        ("statusCode", "HTTP status code", "integer"),
    ]), NO_Default, IS_Obligated)]
)

EventDoc("HTTP.progress", "Event called whenever new data is received (after the headers).",
        SeesDocs("HTTP.error|HTTP.response|HTTP.headers"),
        NO_Examples,
        [
            ParamDoc( "event", "Event object", ObjectDoc([
                ("total","Total bytes (retrieved from the content-length or if no content-length is providen 0) ", "integer"),
                ("read", "Number of bytes read so far", "integer"),
                ("type", "The type of the data returned : `String`, `json`, `binary`", "string"),
                ("data", "Response data", "null|object|object|string|ArrayBuffer")
            ]),  NO_Default, IS_Obligated )
        ]
)
# }}}

