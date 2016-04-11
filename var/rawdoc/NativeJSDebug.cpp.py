# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

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
	SeesDocs( "global.Debug|NativeDebug" ),
	[ExampleDoc( """var d = {a:1, b: "a"};
var s = Debug.serialize(d);
var u = Debug.unserialize(s)
console.log(JSON.stringify(u));
""")],
	IS_Dynamic, IS_Public, IS_Readonly,
	ObjectDoc([]),
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
	[ParamDoc( 'obj', 'object to dump', ObjectDoc([('data', 'content', 'string')]), NO_Default, IS_Obligated ) ],
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
	[	ParamDoc( 'arraybuf', 'Object to dump into an arraybuffer', 'ArrayBuffer', NO_Default, IS_Obligated ),
	 ParamDoc( 'offset', 'offset to start on', 'integer', 0, IS_Optional ) ],
	ReturnDoc( "exerpt of dumped data", "mixed" )
)

