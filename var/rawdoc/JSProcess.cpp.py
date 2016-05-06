# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "NidiumProcess", "Object providing various information and interfaces to interact with system process.",
	SeesDocs( "global.process|NidiumProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	NO_Inherrits,
	NO_Extends,
)

FieldDoc( "global.process", "Instance of the the `NidiumProcess` class.",
	SeesDocs( "global.process|NidiumProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	ObjectDoc([]),
	NO_Default
)

FieldDoc( "NidiumProcess.argv", "The arguments that were set upon construction.",
	SeesDocs( "NidiumProcess" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	ObjectDoc([]),
	NO_Default
)

FieldDoc( "NidiumProcess.workerId", "The identifier for the current worker.",
	SeesDocs( "NidiumProcess|Threads" ),
	[ExampleDoc("""console.log(JSON.stringify(process));""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FunctionDoc("NidiumProcess.setOwner", """Set a user and optionaly a group to a process.

The `setOwner` function is permitted if the effective user/group name is that of the super user, or if the specified user/group name is the same as the effective user/group name.

On failure this function throws an exception.""",
	SeesDocs("NidiumProcess.getOwner"),
	[ExampleDoc("""process.setOwner('daemon', 'www-data');""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[
            ParamDoc("user", "User whom should run this process", 'string|int', NO_Params, IS_Obligated),
	    ParamDoc("group", "Group whom should run this process", 'string|int', NO_Params, IS_Optional)
        ],
	NO_Returns
)

FunctionDoc("NidiumProcess.getOwner", """Return an object with information about the owner of the process.""",
	SeesDocs("NidiumProcess.setOwner"),
	[ExampleDoc("""process.setOwner('daemon', 'www-data');""")],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	ObjectDoc([
	    ("uid", "User ID", "integer"), 
	    ("gid", "Group ID", "integer"), 
	    ("user", "User name", "string"), 
	    ("group", "Group name", "string"), 
	])
)

FunctionDoc("NidiumProcess.setSignalHandler", "Attach a javascript callback to a signal.",
	SeesDocs("NidiumProcess|Threads|NidiumProcess.setSignalHandler|NidiumProcess.exit"),
	[ExampleDoc("""process.setSignalHandler(function(){
console.log("got Kill");
})
""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[CallbackDoc("cb", "function to start if the process receives a signal", NO_Params)],
	NO_Returns
)

FunctionDoc("NidiumProcess.exit", "Attach a javascript callback to a signal.",
	SeesDocs("NidiumProcess|Threads|NidiumProcess.setSignalHandler|NidiumProcess.exit"),
	[ExampleDoc("""var realy = false;
if (realy) {
	process.exit();
}""")],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

