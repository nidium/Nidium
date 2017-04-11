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
    [
        SeeDoc("Keyboard.show"),
        SeeDoc("Keyboard.OPTION_NORMAL"),
        SeeDoc("Keyboard.OPTION_NUMERIC"),
        SeeDoc("Keyboard.OPTION_TEXT_ONLY")
    ],
    [ExampleDoc("var Keyboard= require(\"Keyboard\");\nKeyboard.show();")],
    IS_Static,
    params=[
        ParamDoc("options", "Virtual keyboard options", "integer", NO_Default, IS_Optional),
    ]
)

FunctionDoc("Keyboard.hide", "Hide the virtual keyboard",
    [SeeDoc("Keyboard.show")],
    [ExampleDoc("var Keyboard= require(\"Keyboard\");\nKeyboard.show();\nsetTimeout(function() { Keyboard.hide() }, 3000);")],
    IS_Static,
    params=NO_Params
)

FieldDoc("Keyboard.OPTION_NORMAL", "Normal android keyboard (default)",
    [SeeDoc("Keyboard.show")],
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    "integer"
)

FieldDoc("Keyboard.OPTION_NUMERIC", "Numeric only keyboard",
    [SeeDoc("Keyboard.show")],
    [ExampleDoc("var Keyboard= require(\"Keyboard\");\nKeyboard.show(Keyboard.OPTION_NUMERIC);")],
    IS_Static, IS_Public, IS_Readonly,
    "integer"
)

FieldDoc("Keyboard.OPTION_TEXT_ONLY", "Text only virtual keyboard (no auto-completion, emoji, etc..)",
    [SeeDoc("Keyboard.show")],
    NO_Examples,
    IS_Static, IS_Public, IS_Readonly,
    "integer"
)
