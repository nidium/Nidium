from dokumentor import *

NamespaceDoc( "NativeProfile", "Profile some js code.",
	NO_Sees,
	NO_Examples
)

FunctionDoc( "NativeProfile.toJSObject", "Transform into a Javascript object.",
	[ SeeDoc( "NativeDebug" ), SeeDoc( "NativeProfile.toJSObject" ), SeeDoc( "NativeProfile.dump" ) ],
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "profile information", ObjectDoc([]) )
)

FunctionDoc( "NativeProfile.dump", "Transform a 'string' into a Javascript object.",
	[ SeeDoc( "NativeDebug" ), SeeDoc( "NativeProfile.toJSObject" ), SeeDoc( "NativeProfile.dump" ) ],
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "tmp", "data", 'string', IS_Obligated ) ],
	ReturnDoc("obj", ObjectDoc([]))
)

