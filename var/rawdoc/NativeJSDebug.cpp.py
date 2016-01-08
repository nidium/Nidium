from dokumentor import *

NamespaceDoc( "NativeDebug", "Debugging helper functions", 
	NO_Sees,
	NO_Examples,
)

FunctionDoc( "NativeDebug.serialize", "Dump the content of an object",
	[ SeeDoc( "NativeDebug.unserialize" ) ],
	ExampleDoc( """var d = NativeDebug( { wtf: 'nidium' }  );""" ), 
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( 'object', 'object to dump', 'object', NO_Default, IS_Obligated ) ],
	ReturnDoc( "list with dumped data", "[mixed]" )
)

FunctionDoc( "NativeDebug.unserialize", "Dump the content of an object",
	[ SeeDoc( "NativeDebug.serialize" ) ],
	ExampleDoc( """var d = NativeDebug( { wtf: 'nidium' }  );""" ), 
	IS_Static, IS_Public, IS_Fast,
	[	ParamDoc( 'object', 'object to dump', 'object', NO_Default, IS_Obligated ),
	 ParamDoc( 'offset', 'offset to start on', 'integer', 0, IS_Optional ) ],
	ReturnDoc( "exerpt of dumped data", "mixed" )
)
