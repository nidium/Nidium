# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc("DebuggerCompartment", """For debugging purpose, Nidium use the <a href="https://developer.mozilla.org/en-US/docs/Tools/Debugger-API/Debugger">Debugger of SpiderMonkey</a>.

As the Debugger is not residing in the same <a href="https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartmentshttps://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/Compartments">compartement</a> you need to use a DebuggerCompartment to initialize and execute code inside the Debugger""",
    [ SeeDoc( "DebuggerCompartment.run" ) ],
    NO_Examples,
    products=["Frontend", "Server"]
)

ConstructorDoc("DebuggerCompartment", "Create a new instance of a <a href=\"https://developer.mozilla.org/en-US/docs/Tools/Debugger-API/Debugger\">Debugger</a> insde a new compartment",
    sees=[ SeeDoc( "DebuggerCompartment.run" ) ],
    returns=ReturnDoc( "Return a DebuggerCompartment", "DebuggerCompartment" )
)

FunctionDoc("DebuggerCompartment.run", """Run a function inside the Debugger compartement.

Important note : You cannot share variables between compartements as you usually do in JS. Each compartement is like a sandbox. If you want to have access to variables from the main compartement, you can pass them as an arguments to the run() method. Nidium will wrap your variables to make them accesible inside the Debugger compartement.
The compartement of the debugger only expose the console API. None of Nidium APIs will be available.""",
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
        CallbackDoc( 'callback', 'function to be executed in the Debugger compartement',
            [ParamDoc("debugger", "Debugger instance", "DebuggerCompartment", NO_Default, IS_Obligated),
            ParamDoc("params", "arguments", "[any]", NO_Default, IS_Optional)]),
        ParamDoc( 'argn', 'Optional variable to wrap into the Debugger compartement. The wrapped variable is passed as an argument of the callback', 'any', 0, IS_Optional )
    ],
    ReturnDoc( "The value returned from the callback function", "any" )
)
