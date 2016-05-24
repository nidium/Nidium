# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Stream", "Class to work with IO streams.",
    [ SeeDoc( "File" ), SeeDoc( "Http" ) ],
    NO_Examples,
    NO_Extends,
    NO_Inherrits
)

FieldDoc( "Stream.fileSize", "The size of the stream in bytes.",
    [ SeeDoc( "Stream" ), SeeDoc( "File" ), SeeDoc( "Http" ) ],
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    'integer',
    NO_Default
)

FunctionDoc( "Stream.seek", "Go to a position in the stream, counted from the beginning of the stream.",
    SeesDocs( "Stream|File|Stream.seek|Stream.start|Stream.stop|Stream.getNextPacket" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "pos", "Position to move to", "integer", 0, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Stream.start", "Start reading on a stream.",
    SeesDocs( "Stream|File|Stream.seek|Stream.start|Stream.stop|Stream.getNextPacket" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "packetlen", "Read maximal 'packetlen' bytes on the buffer", "integer", 4096, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Stream.getNextPacket", "Continue reading on a stream.",
    SeesDocs( "Stream|File|Stream.seek|Stream.start|Stream.stop|Stream.getNextPacket" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "Returns null if there is nothing more to read; Returns an the read bytes upon success.", "null|Stream" )
)

ConstructorDoc( "Stream", "Creates a new stream instance. A Stream is an IO interface that can perform operation like 'Stream.read' and 'Stream.write'.",
    NO_Sees,
    [ ExampleDoc("""var s = new Stream( "http://www.nidium.com" );
s.onavailabledata = function( ) {
    s.seek( 80 );
    s.stop( );
}
s.start( 1024 );
""" ) ],
    [ ParamDoc( "url", "A stream starts with a scheme ('file://', 'http://' etc.)", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "Stream instance", "Stream" )
)

EventDoc( "Stream.onProgress", "Function that will be called when there is action on the Stream.",
    NO_Sees,
    NO_Examples,
    [ ParamDoc( "buffered", "The number of bytes that were buffered", "integer", NO_Default, IS_Obligated ),
      ParamDoc( "total", "The number of bytes that were read", "integer", NO_Default, IS_Obligated ),
    ]
)

EventDoc( "Stream.onavailabledata", "Function that will be called when new data is available on the Stream.",
    NO_Sees,
    NO_Examples,
    NO_Params,
)

