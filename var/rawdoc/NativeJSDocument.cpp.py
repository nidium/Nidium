from dokumentor import *

NamespaceDoc( "NativeDocument", "Document class.",
	SeesDocs( "Window|Canvas" ),
	NO_Examples
)

FunctionDoc( "NativeDocument.parseNML", "Parses a NML string and sets this as a document.",
	SeesDocs( "NativeDocument.parseNML|NativeDocument.stylesheet|NativeDocument.loadFont" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "ast", "A NML string", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "A document instance", "NativeDocument" )
)

FunctionDoc( "NativeDocument.getCanvasById", "Select an canvas element for further manipulation.",
	SeesDocs("NativeDocument.getScreenData|NativeDocument.getCanvasById" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "id", "Canvas identifier", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "The selected element or null", "Canvas" )
)

FunctionDoc( "NativeDocument.getScreenData", "Provides information about an canvas object.",
	SeesDocs("NativeDocument.getScreenData|NativeDocument.getCanvasById" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "element", "Canvas instance", "Canvas", NO_Default, IS_Obligated ) ],
	ReturnDoc( "dimensions", ObjectDoc([("width", "The width", "integer"),
										("height", "The height", "integer"),
										("data", "The content", "ArrayBuffer")]))
)

FunctionDoc( "NativeDocument.getPasteBuffer", "Get the content of the paste buffer.",
	SeesDocs( "NativeDocument.getPasteBuffer|NativeDocument.setPasteBuffer" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "content", "PasteBuffer text", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "NativeDocument.setPasteBuffer", "Set the content of the paste buffer.",
	SeesDocs( "NativeDocument.getPasteBuffer|NativeDocument.setPasteBuffer" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Paste buffer contents", "string")
)

FunctionDoc( "NativeDocument.showFPS", "Display the number of frames per second.",
	SeesDocs( "NativeDocument.showFPS|NativeDocument.run" ),
	[ExampleDoc("""var canvas = new Canvas(200, 100);
var ctx = canvas.getContext("2d");
document.showFPS(true);
""")],
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "show", "Show it or hide it", "boolean", "false", IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "NativeDocument.run", "Run the application.",
	SeesDocs( "NativeDocument.showFPS|NativeDocument.run" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "location", "Location", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FieldDoc( "NativeDocument.stylesheet", "The Native Style sheet (NSS) that belongs to this document.",
	SeesDocs( "NativeDocument.parseNML|NativeDocument.stylesheet|NativeDocument.loadFont" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	ObjectDoc([]),
	NO_Default
)

FunctionDoc( "NativeDocument.loadFont", "Load a certain font.",
	SeesDocs( "NativeDocument.parseNML|NativeDocument.stylesheet|NativeDocument.loadFont" ),
	[ExampleDoc("""document.loadFont({
    file: "private://assets/fonts/onesize.ttf",
    name: "OneSize"
});
"""), ExampleDoc("""document.loadFont({
    file : "private://modules/fontawesome/fontawesome.ttf",
    name : "fontAwesome"
});""")],
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "name", "Fontname or font filename", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "success", "boolean" )
)

