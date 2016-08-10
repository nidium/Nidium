# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "OS", "Operating system-related utility method",
    NO_Sees,
    NO_Examples,
    products=["Frontend", "Server"]
)

FieldDoc("OS.language", "The language of the OS.",
    examples=[ExampleDoc("var OS = require(\"OS\");\nconsole.log(OS.language)")],
    is_static=True,
    products=["Frontend"] 
)

FieldDoc("OS.platform", """The platform running Nidium

Possible values are : 
* linux
* win
* mac
* android
* ios
* bsd""",
    examples=[ExampleDoc("var OS = require(\"OS\");\nconsole.log(OS.platform)")],
    is_static=True,
)

