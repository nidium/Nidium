from dokumentor import *

ClassDoc( "NativeProcess", "Process information how nidium was started.",
	SeesDocs( "global.process|NativeProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	NO_Inherrits,
	NO_Extends,
)

FieldDoc( "global.process", "Instance of the the `NativeProcess` class.",
	SeesDocs( "global.process|NativeProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	ObjectDoc([]),
	NO_Default
)

FieldDoc( "NativeProcess.argv", "The arguments that were set upon construction.",
	SeesDocs( "NativeProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	ObjectDoc([]),
	NO_Default
)

FieldDoc( "NativeProcess.workerId", "The identifier for the current worker.",
	SeesDocs( "NativeProcess|Threads" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FunctionDoc("NativeProcess.setSignalHandler", "Attach a javascript callback to a signal.",
	SeesDocs("NativeProcess|Threads|NativeProcess.setSignalHandler|NativeProcess.exit"),
	[ExampleDoc("""process.setSignalHandler(function(){
console.log("got Kill");
})
""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[CallbackDoc("cb", "function to start if the process receives a signal", NO_Params)],
	NO_Returns
)

FunctionDoc("NativeProcess.exit", "Attach a javascript callback to a signal.",
	SeesDocs("NativeProcess|Threads|NativeProcess.setSignalHandler|NativeProcess.exit"),
	[ExampleDoc("""process.exit();""")],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

