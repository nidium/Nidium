from dokumentor import *

FunctionDoc("NativeProcess.setUser", "Set a user or a group to a process.",
	SeesDocs("NativeProcess|Threads|NativeProcess.setSignalHandler|NativeProcess.exit"),
	[ExampleDoc("""process.setUser('daemon', 'www-data');""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc("user", "User whom should run this process", 'string', NO_Params, IS_Obligated),
	 ParamDoc("group", "Group om should run this process", 'string', NO_Params, IS_Optional)],
	NO_Returns
)

