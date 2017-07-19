# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "global", "Global helper functions.",
    NO_Sees,
    [
        ExampleDoc( """var filename = 'script.js';
try {
    load(filename);
} catch(e) {
    console.log("Could not open '" + filename + "': " + e.message)
}"""),
        ExampleDoc( """console.log( process.cwd( )  + '\\n' + __dirname + '\\n' + __filename + '\\n' );"""),
        ExampleDoc( """var t = setTimeout( function() {
    console.log( "Nidium" );}, 1000 );
clearTimeout( t );"""),
        ExampleDoc( """var t = setInterval( console.log, 1000, "Nidium" );
clearInterval( t );"""),
    ],
    products=["Server", "Frontend"]
)

FieldDoc( "global.__dirname", "The path (without the filename) of the current JavaScript file ending with a '/'.",
    [ SeeDoc( "global.__filename" ), SeeDoc( "process.cwd" ), SeeDoc( "fs" ), SeeDoc( "File" ) ],
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    'string',
    NO_Default
)

FieldDoc( "global.__filename", "The path (with the filename) of the current JavaScript file.",
    [ SeeDoc( "global.__dirname" ), SeeDoc( "process.cwd" ) ],
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    'string',
    NO_Default
)

FieldDoc( "global.window", "The main window object.",
    NO_Sees,
    NO_Examples,
    IS_Static, IS_Public, IS_ReadWrite,
    'Window',
    NO_Default,
    products=["Frontend"]
)

FunctionDoc( "global.load", """Load the specified script in a synchronous way.

The given JavaScript file will be loaded in the global scope. That is, every variables defined in it will be defined on the global scope.

This function is only available if the nidium application is running a NML file that was loaded from a local source (e.g. file://)
The 'path' is relative to the NML file that run the current application.""",
    [ SeeDoc( "global.require" )],
    [ExampleDoc( """try {
    load( '/nidium.js' );
} catch(e) {
    console.log("Failed to load : "+ e.message);
}""" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "path", "The javascript sourcefile that needs to be imported", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "global.setTimeout", "Calls a function after a time delay has passed.",
    SeesDocs( "global.setImmediate|global.setTimeout|global.clearTimeout|global.setInterval" ),
    [ ExampleDoc( """var t = setTimeout ( console.log, 1000, "Nidium" );
clearTimeout( t );""" ) ],
    IS_Static, IS_Public, IS_Fast,
    [
        CallbackDoc( 'fn', 'The function to be called', NO_Default ),
        ParamDoc( 'timeout','timeout in milliseconds', 'integer', NO_Default, IS_Obligated ),
        ParamDoc( 'args', "argument for the callback", 'any', NO_Default, IS_Obligated )
    ],
    ReturnDoc( "An identifier representing the timer that can be used to stop the timer with 'global.clearTimeout'", 'integer' )
)

FunctionDoc( "global.setInterval", "Calls a function repeatedly, with a fixed time delay between each call to that function.",
    SeesDocs( "global.setImmediate|global.setTimeout|global.clearTimeout|global.setInterval" ),
    [ ExampleDoc( """var t = setInterval( function() {
    console.log( "Nidium" );
}, 1000 );
clearInterval( t );""") ],
    IS_Static, IS_Public, IS_Fast,
    [
        CallbackDoc( 'fn', 'The function to be called', NO_Default ),
        ParamDoc( 'timeout','timeout in milliseconds', 'integer', 8, IS_Obligated ),
        ParamDoc( 'args', 'mixed', '[any]', IS_Optional )
    ],
    ReturnDoc( "An identifier representing the timer that can be used to stop the timer with 'global.clearInterval'", 'integer' )
)

FunctionDoc( "global.clearTimeout", "Stop a specified timeout timer.",
    SeesDocs( "global.setImmediate|global.setTimeout|global.clearTimeout|global.setInterval" ),
    [ ExampleDoc( """var t = setTimeout( function() {
    console.log( "Nidium" );
}, 1000 );
clearTimeout( t ); """) ],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( 'identifier', "Timer identifier returned by 'global.setTimeout'.", 'integer', NO_Default, IS_Obligated )],
    NO_Returns
)

FunctionDoc( "global.clearInterval", "Stop a specified interval timer.",
    SeesDocs( "global.setImmediate|global.setTimeout|global.clearTimeout|global.setInterval" ),
    [ ExampleDoc( """var t = setInterval( function() {
    console.log( "Nidium" );
}, 1000 );
clearInterval( t );""") ],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( 'identifier', "Timer identifier returned by 'global.setInterval'.", 'integer', NO_Default, IS_Obligated )],
    NO_Returns
)

FunctionDoc( "global.setImmediate", """This is meant to replace the old hack of setting a timer to 0ms.

Every 'setImmediate` callback is executed after the I/O completion but before all the other timers.""",
    SeesDocs( "global.setImmediate|global.setTimeout|global.clearTimeout|global.setInterval" ),
    [ ExampleDoc( """var t = setImmediate( function(){
    console.log( "Nidium" );
}); """) ],
    IS_Static, IS_Public, IS_Fast,
    [   CallbackDoc( 'fn', 'The function to be called', NO_Default ),
        ParamDoc( 'args', 'mixed', '[any]', NO_Default, IS_Optional ) ],
    ReturnDoc( "This always returns 'null'; thus it means that it cannot be canceled with a 'global.clearTimeout'.", 'void' )
)

FunctionDoc( "global.btoa", "Encode binary string to an base64 encodedstring.",
    NO_Sees,
    [ ExampleDoc( """console.log(btoa("Hello Nidium"));""") ],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( 'binary', "The binary to encode.", 'string', NO_Default, IS_Obligated )],
    ReturnDoc( "The encoded string on base64", "string" )
)

