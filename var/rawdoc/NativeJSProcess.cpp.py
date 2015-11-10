from dokumentor import *

ClassDoc( "NativeProcess", "Start a process in the background",
	NO_Sees,
	[ ExampleDoc( """var p = NativeProcess( "wget", "-m www.nidium.com" ); """) ],
	NO_Inherrits,
	NO_Extends,
)

ConstructorDoc( "NativeProcess", "Constructor",
	[ SeeDoc( "NativeProcess.args" ) ],
	[ ExampleDoc( """var p = NativeProcess( "wget", "-m www.nidium.com" ); """) ],
	[ ParamDoc( "arg1", "argument 1", 'string', NO_Default, IS_Obligated ),
	  ParamDoc( "arg..", "arguments", 'string', NO_Default, IS_Optional ),
	],
	ReturnDoc( "The created instance", "object" )
)

FieldDoc( "NativeProcess.argv", "The arguments that were set upon construction",
	[ SeeDoc( "NativeProcess" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	'object',
	NO_Default
)

FieldDoc( "NativeProcess.workerId", "The identifier for the current worker",
	[ SeeDoc( "NativeProcess|Threads" ) ],
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FunctionDoc("NativeProcess.setSignalHandler", "Attach a javascript callback to a signal",
	SeesDocs("NativeProcess|Threads|NativeProcess.setSignalHandler|NativeProcess.exit"),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[CallbackDoc("cb", "function to start if the process receives a signal", NO_Params)],
	NO_Returns
)
FunctionDoc("NativeProcess.exit", "Attach a javascript callback to a signal",
	SeesDocs("NativeProcess|Threads|NativeProcess.setSignalHandler|NativeProcess.exit"),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

