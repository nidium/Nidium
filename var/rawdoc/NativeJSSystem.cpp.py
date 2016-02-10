from dokumentor import *

NamespaceDoc( "System", "System statistics.",
	NO_Sees,
	NO_Examples
)

FunctionDoc( "System.getOpenFileStats", "Provide information about the openfiles.",
	NO_Sees,
	[ExampleDoc( "console.log( JSON.stringify( System.getOpenFileStats() ));" )],
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Object with keys: cur/integer, max/integer, open/integer, sockets/integer, files/integer", "Object")
)

