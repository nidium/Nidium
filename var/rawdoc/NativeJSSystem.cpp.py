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
	ReturnDoc( "statistics", ObjectDoc([("cur", "Current open files accourding to rlimit",  "integer"),
										("max", "Maximum open files accourding to rlimit", "integer"),
										("open", "Open files", "integer"),
										("sockets", "Open sockes", "integer"),
										("files", "Open Files", "integer")]))
)

