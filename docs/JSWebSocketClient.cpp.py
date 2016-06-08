# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "WebSocket", "Websocket handling.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    NO_Inherrits,
    NO_Extends,
    section="WebSocket Client & Server",
    products=["Frontend", "Server"]
)

FunctionDoc("WebSocket.close", "Closes an existing websocket connection.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc("WebSocket.send", "Send an message over a websocket connection.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc("message", "Message to be send", "string|ArrayBuffer", NO_Default, IS_Obligated)],
    ReturnDoc("0 on success, or null if failed.", "integer|null")
)

FunctionDoc("WebSocket.ping", "Pings to an existing websocket connection.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ParamDoc("url", "Url  (may start with 'wss://', 'ws://' or ''", "string", NO_Default, IS_Obligated),
     ParamDoc( "protocol", "No-use yet", "string", NO_Default, IS_Optional ) ],
    ReturnDoc("Websocket connection or null on failure", "WebSocket|null")
)

EventDoc( "WebSocketServer.onmessage", "Event triggered when a message arrives.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    [ParamDoc( "client", "The connected client", "WebSocketServerClient", NO_Default, IS_Obligated ),
    ParamDoc( "event", "event object", ObjectDoc([('data', 'The content', 'string')]), NO_Default, IS_Obligated ) ]
)

EventDoc( "WebSocket.onopen", "Event triggered when a client connects.",
    SeesDocs( "WebSocketServerClient|WebSocketServer|WebSocket"),
    NO_Examples,
    [ParamDoc( "client", "The connected client", "WebSocketServerClient", NO_Default, IS_Obligated ) ]
)

