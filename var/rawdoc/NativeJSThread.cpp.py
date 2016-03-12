from dokumentor import *

NamespaceDoc( "_GLOBALThread", "Global threadnamespace.",
	[SeeDoc( "Thread" ), SeeDoc( "ThreadMessageEvent" )  ],
	NO_Examples
)

ClassDoc( "Thread", """Run a CPU intensive task in the background, with out locking the main UI interface.

This will run in a seperate JS Runtime, wich requires some cpu and memory overhead.""",
	[ SeeDoc( "_GLOBALThread" ), SeeDoc( "ThreadMessageEvent" ) ],
	[ExampleDoc( """document.status.open();
	var t = new Thread(function(foo){
   // something loud and heavy
});
t.oncomplete = function(event){
   // executed when the job is done
   console.log(event.data);
};
t.start("bar"); // start the new job with "bar" as a parameter.""") ],
	NO_Inherrits,
	NO_Extends
)

ClassDoc( "ThreadMessageEvent", "Communication channel between threads.",
	[ SeeDoc( "_GLOBALThread" ), SeeDoc( "Thread" ) ],
	NO_Examples,
	NO_Inherrits,
	NO_Extends
)

FunctionDoc( "Thread.start", "Start a tread.",
	SeesDocs( "Thread|ThreadMessageEvent" ),
	[ ExampleDoc( """document.status.open();

var t = new Thread(function(...n){
    var p = 0;
    for (var i = 0; i < 20000000; i++) {
        if (i % 10000 == 0) this.send(i);
        p++;
    }
    return n;
});
t.onmessage = function(event){
    var i = event.data,
        v = i * 100 / 20000000;

    document.status.label = Math.round(v) + "%";
    document.status.value = v;
};
t.oncomplete = function(event){
    if (event.data){
        console.log("i'm done with", event.data);
    }
    document.status.close();
};
t.start(5, 6, 6, 9);
""") ],
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "args", "function arguments", "mixed", IS_Optional ) ],
	NO_Returns
)

EventDoc( "Thread.onmessage", "Function that will be called when the thread posts a message.",
	SeesDocs( "Thread.oncomplete|Thread.onmessage" ),
	NO_Examples,
	[ ParamDoc( "event", "event object", ObjectDoc([("data", "The message data", "string")]), NO_Default, IS_Obligated ) ]
)

EventDoc( "Thread.oncomplete", "Function that will be called when the thread is complete.",
	SeesDocs( "Thread.oncomplete|Thread.onmessage" ),
	NO_Examples,
	[ ParamDoc( "event", "event object", ObjectDoc([("data", "The message data", "string")]), NO_Default, IS_Obligated ) ]
)

FunctionDoc( "_GLOBALThread.send", "Send a message to a thread.",
	NO_Sees,
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

