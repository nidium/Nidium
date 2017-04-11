# Copyright 2017 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("Keyboard", "Manage virtual keyboard state on mobile devices",
    NO_Sees,
    NO_Examples,
    products=["Frontend"]
)

FunctionDoc("Keyboard.show", "Show the virtual keyboard",
    [SeeDoc("Keyboard.hide")],
    [ExampleDoc("var Keyboard= require(\"Keyboard\");\nKeyboard.show();")],
    IS_Static,
    params=NO_Params,
    products=["Frontend"]
)

FunctionDoc("Keyboard.hide", "Hide the virtual keyboard",
    [SeeDoc("Keyboard.show")],
    [ExampleDoc("var Keyboard= require(\"Keyboard\");\nKeyboard.show();\nsetTimeout(function() { Keyboard.hide() }, 3000);")],
    IS_Static,
    params=NO_Params,
    products=["Frontend"]
)
