# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "Console", "Output, profile and logging functions.",
    [SeeDoc("global.console")],
    [ ExampleDoc( "console.log( 'Nidium' );" ) ],
    products=["Frontend", "Server"]
)
FunctionDoc( "Console.hide", "Hides the console.",
    [ SeeDoc( "Console.show" ) ],
    [ExampleDoc( "console.hide( );" )],
    IS_Static, 
    products=["Frontend"]
)

FunctionDoc( "Console.show", "Opens the console.",
    [ SeeDoc( "Console.hide" ) ],
    [ExampleDoc( "console.show( );" )],
    IS_Static,
    products=["Frontend"]
)

FunctionDoc( "Console.clear", "Clears the console.",
    [ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ) ],
    [ExampleDoc( "console.clear( );" )],
    IS_Static,
    products=["Frontend"]
)

FunctionDoc( "Console.log", "Logs output to the console.",
    [ SeeDoc( "Console.info" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
    [ExampleDoc( "console.log( 'Nidium' );" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
    NO_Returns
)

FunctionDoc( "Console.info", "Logs output to the console, marked as info.",
    [ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
    [ExampleDoc( "console.info( 'Nidium, A new breed of browser' );" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
    NO_Returns
)

FunctionDoc( "Console.error", "Logs output to the console, marked as error.",
    [ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
    [ExampleDoc( "console.error( 'Nidium says \"no\"' );" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
    NO_Returns
)

FunctionDoc( "Console.warn", "Logs output to the console, marked as warn.",
    [ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.info" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
    [ExampleDoc( "console.warn( 'Nidium, Improving the web' );" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
    NO_Returns
)

FunctionDoc( "Console.write", """Log output to the console without processing.

The string printed will not contain the JS file or line number and new line at the end of the string.""",
    [ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.clear" ) ],
    [ExampleDoc( "console.write( 'Nidium', 'A new breed of browser' );" )],
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional ) ],
    NO_Returns
)
