# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("DebuggerCompartment", """For debugging purpose, Nidium use the <a href="https://developer.mozilla.org/en-US/docs/Tools/Debugger-API/Debugger">Debugger of SpiderMonkey</a>.

As the Debugger is not residing in the same <a href="https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartmentshttps://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartments">compartment</a> you need to use a DebuggerCompartment to initialize and execute code inside the Debugger""",
    [ SeeDoc( "DebuggerCompartment.run" ) ],
    NO_Examples,
    products=["Frontend", "Server"]
)

ConstructorDoc("DebuggerCompartment", "Create a new instance of a <a href=\"https://developer.mozilla.org/en-US/docs/Tools/Debugger-API/Debugger\">Debugger</a> inside a new compartment",
    sees=[ SeeDoc( "DebuggerCompartment.run" ) ],
    returns=ReturnDoc( "Return a DebuggerCompartment", "DebuggerCompartment" )
)

FunctionDoc("DebuggerCompartment.run", """Run a function inside the Debugger compartment.

Important note : You cannot share variables between compartments as you usually do in JS. Each compartment is like a sandbox. If you want to have access to variables from the main compartment, you can pass them as an arguments to the run() method. Nidium will wrap your variables to make them accessible inside the Debugger compartment.
The compartment of the debugger only expose the console API. None of Nidium APIs will be available.""",
    NO_Sees,
    [ExampleDoc( """var dbg = new DebuggerCompartment();
dbg.run(function(dbg) {
    // dbg is the Debugger instance
    dbg.onEnterFrame = function(frame) {
        console.log("Entering inside function " + (frame.callee && frame.callee.displayName ? frame.callee.displayName : "anonymous"));
    }
});

// The main compartment is not shared with the Debugger.
// You cannot use variables from the parent scope.
var foo = "bar";
dbg.run(function(dbg) {
    try {
        console.log(foo); // undefined
    } catch ( e ) {
        console.log("as expected, 'foo' is undefined");
    }
});

// However you can explicitly pass a variable into the Debugger compartment
var foo = "bar";
var obj = {"foo": "bar"};
dbg.run(function(dbg, wrappedFoo, wrappedObj) {
    console.log(wrappedFoo); // bar
    console.log(JSON.stringify(wrappedObj)); // {"foo": "bar"}
}, foo, obj);""" )],
    IS_Static, IS_Public, IS_Fast,
    [   ParamDoc("context", "Debugger compartment context", "DebuggerCompartment", NO_Default, IS_Obligated ),
        CallbackDoc( 'callback', 'function to be executed in the Debugger compartment',
            [ParamDoc("debugger", "Debugger instance", "DebuggerCompartment", NO_Default, IS_Obligated),
            ParamDoc("params", "arguments", "[any]", NO_Default, IS_Optional)]),
        ParamDoc( 'arg', 'Optional variable to wrap into the Debugger compartment. The wrapped variable is passed as an argument of the callback', 'any', 0, IS_Optional )
    ],
    ReturnDoc( "The value returned from the callback function", "any" )
)
