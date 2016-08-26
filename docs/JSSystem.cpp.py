# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *
FieldDoc("global.System", "System instance",
    SeesDocs("NidiumSystem|global|global.System"),
    NO_Examples,
    products=["Frontend", "Server"]
)

NamespaceDoc( "NidiumSystem", "System statistics.",
    SeesDocs("NidiumSystem|global|global.System"),
    NO_Examples,
    products=["Frontend", "Server"]
)

FunctionDoc( "NidiumSystem.getOpenFileStats", "Provide information about the openfiles.",
    SeesDocs("NidiumSystem|global|global.System"),
    [ExampleDoc( "console.log( JSON.stringify( System.getOpenFileStats() ));" )],
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "statistics", ObjectDoc([("cur", "Current open files accourding to rlimit",  "integer"),
                                        ("max", "Maximum open files accourding to rlimit", "integer"),
                                        ("open", "Open files", "integer"),
                                        ("sockets", "Open sockes", "integer"),
                                        ("files", "Open Files", "integer")]))
)
FunctionDoc( "NidiumSystem.language", "Get the system language.",
    SeesDocs("NidiumSystem|global|global.System"),
    [ExampleDoc("console.log(System.language)")],
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

