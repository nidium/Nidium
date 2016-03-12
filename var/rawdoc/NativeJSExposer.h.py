from dokumentor import *

NamespaceDoc( "NativeJSEvent", "Event coordinator.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples
)

EventDoc( "NativeJSEvent.stopProgagation", "Stops the propagation of events.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

EventDoc( "NativeJSEvent.preventDefault", "Prevents that the default event will fire.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

EventDoc( "NativeJSEvent.forcePropagation", "Forces an event to fire.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

FunctionDoc( "NativeJSEvent.fireEvent", "Starts an event.",
	SeesDocs( "NativeJSEvent.addEventListener|NativeJSEvent.fireEvent" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
	  ParamDoc("param", "Native Object on where the event should fire", ObjectDoc([]), NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "NativeJSEvent.addEventListener", "Register an action for an event.",
	SeesDocs( "NativeJSEvent.addEventListener|NativeJSEvent.fireEvent" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
	  CallbackDoc( "callback", "The function to execute once the event occurs", [ParamDoc("param", "Arguments", ObjectDoc([]), NO_Default, IS_Obligated ) ] )],
	NO_Returns
)

