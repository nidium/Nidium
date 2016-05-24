# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "process", "Object providing various information and interfaces to interact with system process.",
    SeesDocs( "global.process|process" ),
    [ExampleDoc("""console.log(JSON.stringify(process));""")],
    NO_Inherrits,
    NO_Extends,
)

FieldDoc( "global.process", "Instance of the the `process` class.",
    SeesDocs( "global.process|process" ),
    [ExampleDoc("""console.log(JSON.stringify(process));""")],
    IS_Dynamic, IS_Public, IS_Readonly,
    ObjectDoc([]),
    NO_Default
)

FieldDoc( "process.argv", "The arguments that were set upon construction.",
    SeesDocs( "process" ),
    [ExampleDoc("""console.log(JSON.stringify(process));""")],
    IS_Dynamic, IS_Public, IS_Readonly,
    ObjectDoc([]),
    NO_Default
)

FieldDoc( "process.workerId", "The identifier for the current worker.",
    SeesDocs( "process|Threads" ),
    [ExampleDoc("""console.log(JSON.stringify(process));""")],
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FunctionDoc("process.setOwner", """Set a user and optionaly a group to a process.

The `setOwner` function is permitted if the effective user/group name is that of the super user, or if the specified user/group name is the same as the effective user/group name.

On failure this function throws an exception.""",
    SeesDocs("process.getOwner"),
    [ExampleDoc("""process.setOwner('daemon', 'www-data');""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [
            ParamDoc("user", "User whom should run this process", 'string|int', NO_Params, IS_Obligated),
        ParamDoc("group", "Group whom should run this process", 'string|int', NO_Params, IS_Optional)
        ],
    NO_Returns
)

FunctionDoc("process.getOwner", """Return an object with information about the owner of the process.""",
    SeesDocs("process.setOwner"),
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

FunctionDoc("process.setSignalHandler", "Attach a javascript callback to a signal.",
    SeesDocs("process|Threads|process.setSignalHandler|process.exit"),
    [ExampleDoc("""process.setSignalHandler(function(){
console.log("got Kill");
})
""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc("cb", "function to start if the process receives a signal", NO_Params)],
    NO_Returns
)

FunctionDoc("process.exit", "Attach a javascript callback to a signal.",
    SeesDocs("process|Threads|process.setSignalHandler|process.exit"),
    [ExampleDoc("""var realy = false;
if (realy) {
    process.exit();
}""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "process.cwd", "Get the current working directory.",
    [ SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ), SeeDoc( "File.isDir" ), SeeDoc( "File.rmrf" ), SeeDoc( "File.listFiles" ), SeeDoc( "fs" ), SeeDoc( "process.cwd" ) ],
    [ExampleDoc( """console.log( cwd( ) );""" )],
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The current path", "string" )
)


