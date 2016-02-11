from dokumentor import *
NamespaceDoc( "NativeDebug", "Debugging helper functions.",
	NO_Sees,
	[ExampleDoc( """var d = {a:1, b: "a"};
var s = Debug.serialize(d);
var u = Debug.unserialize(s)
console.log(JSON.stringify(u));
""")]
)

FieldDoc( "global.Debug", "Interfaces to the NativeDebug class.",
	[ SeeDoc( "global.Debug|NativeDebug" ) ],
	[ExampleDoc( """var d = {a:1, b: "a"};
var s = Debug.serialize(d);
var u = Debug.unserialize(s)
console.log(JSON.stringify(u));
""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	'object',
	NO_Default
)

FunctionDoc( "NativeDebug.serialize", "Dump the content of an object.",
	[ SeeDoc( "NativeDebug.unserialize" ) ],
	[ExampleDoc( """var d = {a:1, b: "a"};
var s = Debug.serialize(d);
var u = Debug.unserialize(s)
console.log(JSON.stringify(u));
""")],
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( 'object', 'object to dump', 'object', NO_Default, IS_Obligated ) ],
	ReturnDoc( "list with dumped data", "[mixed]" )
)

FunctionDoc( "NativeDebug.unserialize", "Dump the content of an object.",
	[ SeeDoc( "NativeDebug.serialize" ) ],
	[ExampleDoc( """var d = {a:1, b: "a"};
var s = Debug.serialize(d);
var u = Debug.unserialize(s)
console.log(JSON.stringify(u));
""")],
	IS_Static, IS_Public, IS_Fast,
	[	ParamDoc( 'object', 'object to dump', 'ArrayBuffer', NO_Default, IS_Obligated ),
	 ParamDoc( 'offset', 'offset to start on', 'integer', 0, IS_Optional ) ],
	ReturnDoc( "exerpt of dumped data", "mixed" )
)

