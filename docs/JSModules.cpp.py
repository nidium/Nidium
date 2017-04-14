# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "Module", "Module that is exported.",
    SeesDocs( "global.modules" ),
    examples=[
        ExampleDoc("module.exports = {\"lorem\":\"ipsum\"}"),
        ExampleDoc("""var foobar = require(\"foobar.js\");\n console.log(foobar.lorem); // Print \"ipsum\" """, run_code=False)
    ],
    products=["Frontend", "Server"]
)

FieldDoc( "Module.exports", "Exported Module.",
    SeesDocs( "Module|global.modules" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    "any",
    NO_Default
)

FieldDoc( "Module.id", "The module name.",
    SeesDocs( "Module" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FunctionDoc( "global.require", """`require` implementation similar to NodeJS `require()`""",
    SeesDocs( "Module|global.load" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "path", "modulename", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "modulename", "string" )
)

FieldDoc( "global.modules", "Exposed Modules.",
    SeesDocs( "Module" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    '[Module]',
    NO_Default
)
