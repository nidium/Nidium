from dokumentor import *

NamespaceDoc( "Console", "Output, profile and logging functions.",
	NO_Sees,
	[ ExampleDoc( "console.log( 'Nidium' );" ) ]
)

FunctionDoc( "Console.profile", "Starts a profiler instance.",
	[ SeeDoc( "Console.profileEnd|global.console" ) ],
	ExampleDoc( """console.profile( );
	// Time intense task
	var result = Console.profileEnd( );
	console.log( JSON.stringify( result ) ); """),
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "Console.profileEnd", "Stops a profiler instance.",
	[ SeeDoc( "Console.profile|global.console" ) ],
	ExampleDoc( """console.profile( );
	// Time intense task
	var result = Console.profileEnd( );
	console.log( JSON.stringify( result ) ); """),
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Profile results", "Object" )
)

FunctionDoc( "Console.log", "Logs output to the console.",
	[ SeeDoc( "Console.info" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
	ExampleDoc( "console.log( 'Nidium' );" ),
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
	NO_Returns
)
FunctionDoc( "Console.info", "Logs output to the console, marked as info.",
	[ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
	ExampleDoc( "console.info( 'Nidium, A new breed of browser' );" ),
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
	NO_Returns
)
FunctionDoc( "Console.error", "Logs output to the console, marked as error.",
	[ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.warn" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
	ExampleDoc( "console.error( 'Nidium, Cannot display HTML' );" ),
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
	NO_Returns
)
FunctionDoc( "Console.warn", "Logs output to the console, marked as warn.",
	[ SeeDoc( "Console.log" ), SeeDoc( "Console.info" ), SeeDoc( "Console.info" ), SeeDoc( "Console.write" ), SeeDoc( "Console.clear" ) ],
	ExampleDoc( "console.warn( 'Nidium, Improving the web' );" ),
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "text", "The string that should be logged", "string", NO_Default, IS_Optional )],
	NO_Returns
)

