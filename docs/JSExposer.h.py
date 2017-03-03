# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "NidiumEvent", "Event coordinator.\n\n Every object exposed by Nidium inherits from NidiumEvent.",
    SeesDocs( "NidiumEvent.stopPropagation|NidiumEvent.preventDefault|NidiumEvent.forcePropagation" ),
    products=["Frontend", "Server"]
)

FunctionDoc( "NidiumEvent.stopPropagation", "Stops the propagation of an event.",
    SeesDocs( "NidiumEvent.stopPropagation|NidiumEvent.preventDefault|NidiumEvent.forcePropagation" )
)

FunctionDoc( "NidiumEvent.preventDefault", "Prevents further listener to receive this event.",
    SeesDocs( "NidiumEvent.stopPropagation|NidiumEvent.preventDefault|NidiumEvent.forcePropagation" )
)

FunctionDoc( "NidiumEvent.forcePropagation", "Forces further listener to receive this event.",
    SeesDocs( "NidiumEvent.stopPropagation|NidiumEvent.preventDefault|NidiumEvent.forcePropagation" )
)

"""
FunctionDoc( "NidiumEvent.fireEvent", "Fire the event on all the listener of the object.",
    SeesDocs( "NidiumEvent.addEventListener|NidiumEvent.fireEvent" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
      ParamDoc("param", "Object on where the event should fire", ObjectDoc([]), NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "NidiumEvent.addEventListener", "Register a callback for an event.",
    SeesDocs( "NidiumEvent.addEventListener|NidiumEvent.fireEvent" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "eventName", "The event name", "string", NO_Default, IS_Obligated),
      CallbackDoc( "callback", "The function to execute once the event occurs", [ParamDoc("param", "Arguments", ObjectDoc([]), NO_Default, IS_Obligated ) ] )],
    NO_Returns
)
"""
