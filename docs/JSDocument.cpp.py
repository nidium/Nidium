# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "document", "Document class.",
    SeesDocs( "Window|Canvas" ),
    NO_Examples,
    products=["Server"]
)

FunctionDoc( "document.parseNML", "Parses a NML string and sets this as a document.",
    SeesDocs( "document.parseNML|document.stylesheet|document.loadFont" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "ast", "A NML string", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "A document instance", "document" )
)

FunctionDoc( "document.getCanvasById", "Select an canvas element for further manipulation.",
    SeesDocs("document.getScreenData|document.getCanvasById" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "id", "Canvas identifier", "string", NO_Default, IS_Obligated ) ],
    ReturnDoc( "The selected element or null", "Canvas" )
)

FunctionDoc( "document.getScreenData", "Provides information about an canvas object.",
    SeesDocs("document.getScreenData|document.getCanvasById" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "element", "Canvas instance", "Canvas", NO_Default, IS_Obligated ) ],
    ReturnDoc( "dimensions", ObjectDoc([("width", "The width", "integer"),
                                        ("height", "The height", "integer"),
                                        ("data", "The content", "ArrayBuffer")]))
)

FunctionDoc( "document.getPasteBuffer", "Get the content of the paste buffer.",
    SeesDocs( "document.getPasteBuffer|document.setPasteBuffer" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "content", "PasteBuffer text", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "document.setPasteBuffer", "Set the content of the paste buffer.",
    SeesDocs( "document.getPasteBuffer|document.setPasteBuffer" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "Paste buffer contents", "string")
)

FunctionDoc( "document.showFPS", "Display the number of frames per second.",
    SeesDocs( "document.showFPS|document.run" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
var ctx = canvas.getContext("2d");
document.showFPS(true);
""")],
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "show", "Show it or hide it", "boolean", "false", IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "document.run", "Run the application.",
    SeesDocs( "document.showFPS|document.run" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "location", "Location", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FieldDoc( "document.stylesheet", "The Native Style sheet (NSS) that belongs to this document.",
    SeesDocs( "document.parseNML|document.stylesheet|document.loadFont" ),
    NO_Examples,
    IS_Static, IS_Public, IS_ReadWrite,
    ObjectDoc([]),
    NO_Default
)

FunctionDoc( "document.loadFont", "Load a certain font.",
    SeesDocs( "document.parseNML|document.stylesheet|document.loadFont" ),
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

