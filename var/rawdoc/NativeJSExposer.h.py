# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "NativeJSEvent", "Event coordinator.\n\n Every native object exposed by Nidium inherits from NativeJSEvent.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples
)

EventDoc( "NativeJSEvent.stopProgagation", "Stops the propagation of an event.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

EventDoc( "NativeJSEvent.preventDefault", "Prevents further listener to receive this event.",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

EventDoc( "NativeJSEvent.forcePropagation", "Forces further listener to receive this event..",
	SeesDocs( "NativeJSEvent.stopPropagation|NativeJSEvent.preventDefault|NativeJSEvent.forcePropagation" ),
	NO_Examples,
	NO_Params
)

FunctionDoc( "NativeJSEvent.fireEvent", "Fire the event on all the listener of the object.",
	SeesDocs( "NativeJSEvent.addEventListener|NativeJSEvent.fireEvent" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
	  ParamDoc("param", "Native Object on where the event should fire", ObjectDoc([]), NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "NativeJSEvent.addEventListener", "Register a callback for an event.",
	SeesDocs( "NativeJSEvent.addEventListener|NativeJSEvent.fireEvent" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
	  CallbackDoc( "callback", "The function to execute once the event occurs", [ParamDoc("param", "Arguments", ObjectDoc([]), NO_Default, IS_Obligated ) ] )],
	NO_Returns
)

