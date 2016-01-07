from dokumentor import *

NamespaceDoc( "NativeDocument", "Document class",
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

FunctionDoc( "NativeDocument.getCanvasById", "Select an canvas element for further manipulation",
	SeesDocs("NativeDocument.getScreenData|NativeDocument.getCanvasById" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "id", "Canvas identifier", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "The selected element or null", "Canvas" )
)

FunctionDoc( "NativeDocument.getScreenData", "Provides information about an canvas object",
	SeesDocs("NativeDocument.getScreenData|NativeDocument.getCanvasById" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "element", "Canvas instance", "Canvas", NO_Default, IS_Obligated ) ],
	ReturnDoc( "An object with keys: width/integer, height/integer, data/arrayBuffer", "Object" )
)

FunctionDoc( "NativeDocument.getPasteBuffer", "Get the content of the paste buffer",
	SeesDocs( "NativeDocument.getPasteBuffer|NativeDocument.setPasteBuffer" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "content", "PasteBuffer text", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "NativeDocument.setPasteBuffer", "Set the content of the paste buffer",
	SeesDocs( "NativeDocument.getPasteBuffer|NativeDocument.setPasteBuffer" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Paste buffer contents", "string")
)

FunctionDoc( "NativeDocument.showFPS", "Display the number of frames per second",
	SeesDocs( "NativeDocument.showFPS|NativeDocument.run" ),
	NO_Examples,
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
	"Object",
	NO_Default
)

FunctionDoc( "NativeDocument.loadFont", "Load a certain font.",
	SeesDocs( "NativeDocument.parseNML|NativeDocument.stylesheet|NativeDocument.loadFont" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "name", "Fontname or font filename", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "success", "boolean" )
)
