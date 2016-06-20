# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Image", "Image handling.",
    [SeeDoc( "Video" ) ],
    [ExampleDoc( """document.background = "#333333";
document.opacity = 0;

var pic = new UI.element(document, {
    visible : false,
    width : 1,
    height : 1,
    radius : 6,
    shadowBlur : 6,
    shadowColor : "black"
});

var img = new Image();

img.onload = function(){
    pic.setBackgroundImage(img);
    pic.width = img.width;
    pic.height = img.height;
    pic.visible = true;
    pic.center();
    document.fadeIn(850);
};

img.src = "http://www.nidium.com/static/img/island.png";
""")],
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

