# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "NidiumDocument", """Document class.

It is not possible to create an instance of this class. 'global.document' is already available.""",
    SeesDocs( "Window|Canvas|NidiumDocument|global.document|global.window" ),
    NO_Examples,
    products=["Server"]
)

FunctionDoc( "NidiumDocument.parse", "Parses a NML string and sets this as a document.",
    SeesDocs( "global.document|NidiumDocument.parse|NidiumDocument.stylesheet|NidiumDocument.loadFont" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "ast", "A NML string", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "A document instance", "NidiumDocument" )
)

FunctionDoc( "NidiumDocument.getCanvasById", "Select an canvas element for further manipulation.",
    SeesDocs("global.document|NidiumDocument.getScreenData|NidiumDocument.getCanvasById" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "id", "Canvas identifier", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "The selected element or null", "Canvas", nullable=True )
)

FunctionDoc( "NidiumDocument.getScreenData", "Provides information about an canvas object.",
    SeesDocs("global.document|NidiumDocument.getScreenData|NidiumDocument.getCanvasById" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "element", "Canvas instance", "Canvas", NO_Default, IS_Obligated ) ],
    ReturnDoc( "dimensions", ObjectDoc([("width", "The width", "integer"),
                                        ("height", "The height", "integer"),
                                        ("data", "The content", "ArrayBuffer")]))
)

FunctionDoc( "NidiumDocument.getPasteBuffer", "Get the content of the paste buffer.",
    SeesDocs( "global.document|NidiumDocument.getPasteBuffer|NidiumDocument.setPasteBuffer" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "content", "PasteBuffer text", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "NidiumDocument.setPasteBuffer", "Set the content of the paste buffer.",
    SeesDocs( "global.document|NidiumDocument.getPasteBuffer|NidiumDocument.setPasteBuffer" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "Paste buffer contents", "string")
)

FunctionDoc( "NidiumDocument.showFPS", "Display the number of frames per second.",
    SeesDocs( "global.document|NidiumDocument.showFPS|NidiumDocument.run" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
var ctx = canvas.getContext("2d");
document.showFPS(true);
""")],
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "show", "Show it or hide it", "boolean", "false", IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "NidiumDocument.run", "Run the application.",
    SeesDocs( "global.document|NidiumDocument.showFPS|NidiumDocument.run" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "location", "Location", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FieldDoc( "NidiumDocument.stylesheet", "The Native Style sheet (NSS) that belongs to this document.",
    SeesDocs( "global.document|NidiumDocument.parse|NidiumDocument.stylesheet|NidiumDocument.loadFont" ),
    NO_Examples,
    IS_Static, IS_Public, IS_ReadWrite,
    ObjectDoc([]),
    NO_Default
)

FunctionDoc( "NidiumDocument.loadFont", "Load a certain font.",
    SeesDocs( "global.document|NidiumDocument.parse|NidiumDocument.stylesheet|NidiumDocument.loadFont" ),
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

