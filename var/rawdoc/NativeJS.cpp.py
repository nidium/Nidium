from dokumentor import *

NamespaceDoc( "global", "Global helper functions",
	NO_Sees,
	[ 	ExampleDoc( "load( 'script.js' );"),
		ExampleDoc( """console.log( pwd( )  + ' ' + __dirname + ' ' + __filename );"""),
		ExampleDoc( """var t = setTimeout( 'console.log( "Nidium" );', 1000 );
clearTimeout( t );"""),
		ExampleDoc( """var t = setInterval( 'console.log( "Nidium" );', 1000 );
clearInterval( t );"""),
	]
	)

FieldDoc( "global.__dirname", "The path (without the filename) of the current JavaScript file ending with a '/'.",
	[ SeeDoc( "global.__filename" ), SeeDoc( "global.pwd" ), SeeDoc( "fs" ), SeeDoc( "File" ) ],
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	'string',
	NO_Default
)

FieldDoc( "global.__filename", "The path (with the filename) of the current JavaScript file.",
	[ SeeDoc( "global.__dirname" ), SeeDoc( "global.pwd" ) ],
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	'string',
	NO_Default
)

FieldDoc( "global.window", "The main window object.",
	NO_Sees,
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	'object',
	NO_Default
	)

FunctionDoc( "global.pwd", "Get the current working directory",
	[ SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ), SeeDoc( "File.isDir" ), SeeDoc( "File.rmrf" ), SeeDoc( "File.listFiles" ), SeeDoc( "fs" ), SeeDoc( "global.pwd" ) ],
	ExampleDoc( """console.log( pwd( ) );""" ),
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "The current path", "string" )
)

FunctionDoc( "global.load", """Load the specified script in a synchronous way.

This function is only available if nidium-browser is running a NML file that  was loaded from a local source (e.g. file://)
The 'path' is relative to the NML file that run the current application.""",
	NO_Sees,
	ExampleDoc( """load( '/nidium.js' ); """ ),
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "path", "The javascript sourcefile that needs to be imported", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "global.setTimeout", "Calls a function after a time delay has passed.",
	[ SeeDoc( "global.clearTimeout" ), SeeDoc( "global.clearInterval" ), SeeDoc( "global.setInterval" ) ],
	[ ExampleDoc( """var t = setTimeout ( 'console.log( "Nidium" );', 1000 );
clearTimeout( t );""" ) ],
	IS_Static, IS_Public, IS_Fast,
	[ 	CallbackDoc( 'fn', 'The function to be called', NO_Default ),
 		ParamDoc( 'timeout','timeout in miliseconds', 'integer', 8, IS_Obligated ),
		ParamDoc( 'args', "argument for the callback", 'mixed', '[]', IS_Obligated ) ],
	ReturnDoc( "An identifier representing the timer that can be used to stop the timer with 'global.clearTimeout'", 'integer' )
)

FunctionDoc( "global.setInterval", "Calls a function repeatedly, with a fixed time delay between each call to that function.",
	[SeeDoc( "global.setTimout" ), SeeDoc( "global.clearTimeout" ), SeeDoc( "global.clearInterval" ) ],
	[ ExampleDoc( """var t = setInterval( 'console.log( "Nidium" );', 1000 );
clearInterval( t );""") ],
	IS_Static, IS_Public, IS_Fast,
	[ 	CallbackDoc( 'fn', 'The function to be called', NO_Default ),
 		ParamDoc( 'timeout','timeout in miliseconds', 'integer', 8, IS_Obligated ),
		ParamDoc( 'args', 'mixed', 'Array', IS_Optional ) ],
	ReturnDoc( "An identifier representing the timer that can be used to stop the timer with 'global.clearInterval'", 'integer' )
)

FunctionDoc( "global.clearTimeout", "Stop a specified timeout timer.",
	[ SeeDoc( "global.setTimeout" ), SeeDoc( "global.clearInterval" ), SeeDoc( "global.setInterval" ) ],
	[ ExampleDoc( """var t = setTimeout ( 'console.log( "Nidium" );', 1000 );
clearTimeout( t ); """) ],
	IS_Static, IS_Public, IS_Fast,
	ParamDoc( 'identifier', "Timer identifier returned by 'global.setTimeout'.", 'integer', NO_Default, IS_Obligated ),
	NO_Returns
)

FunctionDoc( "global.clearInterval", "Stop a specified interval timer.",
	[SeeDoc( "global.setTimeout" ), SeeDoc( "global.clearTimeout" ), SeeDoc( "global.setInterval" ) ],
	[ ExampleDoc( """var t = setInterval ( 'console.log( "Nidium" );', 1000 );
clearInterval( t );""") ],
	IS_Static, IS_Public, IS_Fast,
	ParamDoc( 'identifier', "Timer identifier returned by 'global.setInterval'.", 'integer', NO_Default, IS_Obligated ),
	NO_Returns
)
