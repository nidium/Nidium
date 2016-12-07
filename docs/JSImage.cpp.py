# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Image", "Image handling.",
    [SeeDoc( "Canvas.drawImage" ) ],
    [ExampleDoc( """var img = new Image();
var c = new Canvas(200, 200);
var ctx = c.getContext("2d");

document.canvas.add(c);

img.addEventListener("load", function() {
    ctx.drawImage(img, 0, 0);
});                    

img.addEventListener("error", function(ev) {
    console.log(ev.error);
});
                       
img.src = "http://tests.nidium.com/img/nidium.png";""")],
    NO_Extends,
    NO_Inherrits,
    products=["Frontend"]
)

FunctionDoc( "Image.print", "Does nothing.",
    SeesDocs( "Image.shiftHue|Image.markColorInAlpha|Image.desaturate|Image.print" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Image.shiftHue", "Change the hue of an image.",
    SeesDocs( "Image.shiftHue|Image.markColorInAlpha|Image.desaturate|Image.print" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "value", "huevalue", "integer", NO_Default, IS_Obligated ),
      ParamDoc( "color", "color", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Image.markColorInAlpha", "Mark the color in an image.",
    SeesDocs( "Image.shiftHue|Image.markColorInAlpha|Image.desaturate|Image.print" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Image.desaturater", "Desaturate an image.",
    SeesDocs( "Image.shiftHue|Image.markColorInAlpha|Image.desaturate|Image.print" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FieldDoc( "Image.height", "Height of the image in pixels.",
    SeesDocs( "Image.src|Image.heigth|Image.width" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Image.width", "Width of the image in pixels.",
    SeesDocs( "Image.src|Image.heigth|Image.width" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Image.src", "Path to the image files.",
    SeesDocs( "Image.src|Image.heigth|Image.width" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "string",
    "unknown"
)

ConstructorDoc( "Image", "Image loader.",
    NO_Sees,
    NO_Examples,
    NO_Params,
    ReturnDoc( "Image object", "Image" )
)

EventDoc( "Image.load", "Event triggered when the image is loaded.",
    NO_Sees,
    [ExampleDoc(""" var img = new Image();
img.addEventListener("load", function() {
    console.log("Image loaded ! ");
});
img.src = "http://tests.nidium.com/nidium.png";""")],
    [ParamDoc( "event", "Event object", ObjectDoc([]), NO_Default, IS_Obligated ) ]
)

EventDoc( "Image.error", "Event triggered when an error happen durring the loading of the image.",
    NO_Sees,
    [ExampleDoc(""" var img = new Image();
img.addEventListener("error", function() {
    console.log("Image loaded ! ");
});
img.src = "http://www.nidium.com/file_that_does_not_exist.png";""")],
    [ParamDoc( "event", "Event object", ObjectDoc([
            ("error", "Description of the error", "string")
        ]), NO_Default, IS_Obligated) 
    ]
)

