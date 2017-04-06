# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "Canvas", """Canvas object.

A `Canvas` instance is the base surface for any drawing. It is defined by a size and a position.

By default, canvas are exposed without any context (container-only canvas), but before drawing elements can be added to a canvas, the type of context needs to be set.

Nidium is currently supporting the following context types:

* 2D Context is based on the [2DContext API](http://www.w3.org/html/wg/drafts/2dcontext/html5_canvas/).
* A WebGL Context is based on the [WebGL API](https://www.khronos.org/registry/webgl/specs/1.0/).

The 'document' object has a 2D canvas (`document.canvas`) created by default. This is called the "the root canvas".
Every canvas, except the root canvas, has a parent canvas. Only canvas instances that are descendents of the root canvas are visible on the screen.
The parent of a canvas can be set by using methods like `Canvas.add`.""",
    SeesDocs( "global|Window|NativeDocument|WebGL|global.document.canvas|Canvas.add" ),
    [ExampleDoc("""// Create a new 200x100 canvas (logical pixels)
var canvas = new Canvas(200, 100);

// Add it to the root hierarchy
document.canvas.add(canvas);

// Get its 2D context
var ctx= canvas.getContext("2d");

// Draw something into it
ctx.fillStyle = "blue";
ctx.fillRect(0, 0, 100, 100);
""")],
    "CanvasInherrit",
    products=["Frontend"]

)

NamespaceDoc( "CanvasInherrit", "Parent/Prototype object for `Canvas`.",
    [ SeeDoc( "Canvas" ) ],
    NO_Examples,
    section="Canvas"
)

FunctionDoc( "Canvas.show", """Show a previously hidden canvas.""",
    SeesDocs( "Canvas.show|Canvas.hide" ),
    [ExampleDoc( """var canvas = new Canvas(200, 100);
document.canvas.add(canvas);
var ctx = canvas.getContext("2d");
ctx.fillStyle = "blue";
ctx.fillRect(0, 0, 100, 100);
let showIt = true;
setInterval(function(canv) {
    if (showIt) {
        canv.show();
    } else {
        canv.hide();
    }
    showIt = ! showIt;
}, 250, canvas);""" ) ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.hide", "Hides a canvas and all it's children.",
    SeesDocs( "Canvas.show|Canvas.hide" ),
[ExampleDoc( """var canvas = new Canvas(200, 100);
document.canvas.add(canvas);
var ctx = canvas.getContext("2d");
ctx.fillStyle = "blue";
ctx.fillRect(0, 0, 100, 100);
let showIt = true;
setInterval(function(canv) {
    if (showIt) {
        canv.show();
    } else {
        canv.hide();
    }
    showIt = ! showIt;
}, 250, canvas);""" ) ],

    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.clear", "Clears a Canvas by making it transparent.",
    SeesDocs( "Canvas.getContext|Canvas.setContext|Canvas.clear" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
ctx.fillStyle = "red";
ctx.fillRect(10, 10, 100,100);
canvas.clear();""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.setZoom", """Zooms a canvas by a magnifying factor.

The content of the canvas is not redrawn (unlike a 'Canvas.setScale').""",
    SeesDocs( "Canvas.setCoordinates|Canvas.setSize|Canvas.setZoom|Canvas.setScale" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
canvas.setZoom(2);""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "factor", "Zoom factor", "float", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.setScale", "Set a canvas object to a certain scale.",
    SeesDocs( "Canvas.setCoordinates|Canvas.setSize|Canvas.setZoom|Canvas.setScale" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "x_pos", "X position", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "y_pos", "Y position", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.setSize", """Set a canvas object to a certain size.

The operation is slow when the canvas' context is already instantiated with 'Canvas.getContext', because it needs to restructure several things internally.
If the canvas context did not have a `Canvas.getContext` call yet, it is a fast method.""",
    SeesDocs( "Canvas.setCoordinates|Canvas.setSize|Canvas.setZoom|Canvas.setScale" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
canvas.setSize(300, 300);
""")],
    IS_Dynamic, IS_Public, IS_Slow,
    [ ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "height", "Height", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.removeFromParent", """Detach a canvas from it's parent canvas.

If the canvas does not have a parent, this method does nothing.
Only canvas instances that are descendents from the root canvas can be displayed.
The root canvas does NOT have a parent.

The canvas remains in memory as long as it is a valid javascript instance. This way it can be re-attached to another canvas at a later stage.""",
    SeesDocs( "document.canvas|Canvas.add|Canvas.insertBefore|Canvas.insertAfter|Canvas.removeFromParent|document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
canvas.removeFromParent(); """)],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.bringToFront", """Brings a canvas in front of another canvas of the z-order.

This method sets the element's z-index by changing the order of the canvas in the parent's children.""",
    SeesDocs( "Canvas.bringToFront|Canvas.sendToBack|Canvas.getChildren|Canvas.GetParent|Canvas.add|Canvas.getFirstChild|Canvas.getLastChild" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
document.canvas.add(canvas);
var canvas2 = new Canvas(10, 10);
document.canvas.add(canvas2);
canvas.bringToFront();""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.sendToBack", """Sends a canvas to the back of the z-order.

This method sets the element's z-index by changing the order of the canvas in the parent's children.""",
    SeesDocs( "Canvas.bringToFront|Canvas.sendToBack|Canvas.getChildren|Canvas.GetParent|Canvas.add|Canvas.getFirstChild|Canvas.getLastChild" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
document.canvas.add(canvas);

var canvas2 = new Canvas(10, 10);
document.canvas.add(canvas2);

canvas2.sendToBack();""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Canvas.getParent", """Find the parent of a canvas object.

If the canvas has no parent, null is returned.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
    var parent = canvas.getParent();""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The parent object of this instance or 'null'", "Canvas", nullable=True )
)

FunctionDoc( "Canvas.getFirstChild", """Find the first child of a canvas object.

The first child is the direct child with the lowest z-index.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
var child = canvas.getFirstChild();""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The firsth child of this instance or 'null' if there are no children.", "Canvas", nullable=True )
)

FunctionDoc( "Canvas.getLastChild", """Find the last child of a canvas object.",
The last child is the direct child with the highest z-index.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
var child = canvas.getLastChild();""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The last child of this instance or 'null'", "Canvas", nullable=True)
)

FunctionDoc( "Canvas.getNextSibling", """Find the next sibling of a canvas object.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas1 = new Canvas(200, 100);
var canvas2 = new Canvas(200, 100);
var canvas3 = new Canvas(200, 100);
document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var next = canvas2.getNextSibling(); // returns canvas3
var nothing = canvas3.getNextSibling(); // returns null""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The next sibling of this instance or 'null'", "Canvas", nullable=True )
)

FunctionDoc( "Canvas.getPrevSibling", """Find the previous sibling of a canvas object.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas1 = new Canvas(200, 100);
    var canvas2 = new Canvas(200, 100);
    var canvas3 = new Canvas(200, 100);
    document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var prev = canvas2.getPrevSibling(); // returns canvas1
var nothing = canvas1.getPrevSibling(); // returns null""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The previous sibling of this instance or 'null'", "Canvas", nullable=True )
)

FunctionDoc( "Canvas.getChildren", """Find all the children of a canvas object.

Returns an array of children, from the lowest z-index to the highest z-index.
If the canvas does not have any children, it returns an empty array.""",
    SeesDocs("document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas1 = new Canvas(200, 400);
var canvas2 = new Canvas(200, 400);
var canvas3 = new Canvas(200, 400);
document.canvas.add(canvas1);
document.canvas.add(canvas2);
document.canvas.add(canvas3);
var list = document.canvas.getChildren(); // returns [canvas1, canvas2, canvas3]
canvas2.bringToFront(); // brings canvas2 to the front of the display list.
var list = document.canvas.getChildren(); // returns [canvas1, canvas3, canvas2]
""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "The list of children of this instance.", "[Canvas]" )
)

FunctionDoc( "Canvas.getVisibleRect", "Get the outerbounds of the canvas in absolute coordinates.",
    SeesDocs( "Canvas.translate|Canvas.getVisibleRect" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "dimensions", ObjectDoc([("left", "left size", "float"),
                                        ("top", "top size", "float"),
                                        ("width", "width size", "float"),
                                        ("height", "height size", "float")]))
)

FunctionDoc( "Canvas.setCoordinates", """Sets a canvas object at a certain position.

The behavior depends on the 'Canvas.position' type.""",
    SeesDocs( "Canvas.position|Canvas.setCoordinates|Canvas.setSize|Canvas.setZoom|Canvas.setScale|Canvas.left|Canvas.top" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
canvas.position = 'absolute';
canvas.setCoordinates(30, 50);
//alternative way
canvas.left = 30;
canvas.top = 50;""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "left", "Left point", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "right", "Right point", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.translate", "Translate a canvas object.",
    SeesDocs( "Canvas.translate|Canvas.getVisibleRect" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "left", "Left point", "integer", NO_Default, IS_Obligated ),
     ParamDoc( "right", "Right point", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.add", """Add a canvas onto another canvas.

If a canvas is already attached to a parent, it will change it's parent.""",
    SeesDocs( "Canvas.add|Canvas.insertBefore|Canvas.insertAfter|Canvas.removeFromParent|document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
document.canvas.add(canvas);

var canvas2 = new Canvas(10, 10);
canvas.add(canvas2);""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "child", "Canvas instance to add to this instance.", "Canvas", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.insertBefore", "Insert a canvas before another canvas.",
    SeesDocs( "Canvas.add|Canvas.insertBefore|Canvas.insertAfter|Canvas.removeFromParent|document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "insert", "Canvas instance to insert", "Canvas", NO_Default, IS_Obligated ),
     ParamDoc( "reference", "Canvas instance reference", "Canvas", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.insertAfter", "Insert a canvas after another canvas.",
    SeesDocs( "Canvas.add|Canvas.insertBefore|Canvas.insertAfter|Canvas.removeFromParent|document.canvas|Canvas.add|Canvas.getParent|Canvas.getFirstChild|Canvas.getLastChild|Canvas.getNextSibling|Canvas.getPrevSibling" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "insert", "Canvas instance to insert", "Canvas", NO_Default, IS_Obligated ),
     ParamDoc( "reference", "Canvas instance reference", "Canvas", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Canvas.getContext", """Creates a new Canvas context.

The created `Canvas` object provides methods and properties for drawing and manipulating images and graphics on a canvas element.
A context object includes information about colors, line widths, fonts, and other graphic parameters that can be drawn on a canvas.

Nidium is currently supporting the following context types:

* 2D Context is based on the [2DContext API](http://www.w3.org/html/wg/drafts/2dcontext/html5_canvas/).
* A WebGL Context is based on the [WebGL API](https://www.khronos.org/registry/webgl/specs/1.0/).

>* This operation is slow the first time the method is called.
>* The context can't be changed once it's created.
""",
    SeesDocs( "document.canvas|Canvas|Canvas.getContext|Canvas.setContext|Canvas.clear" ),
    [ExampleDoc("""var canvas = new Canvas(200, 100);
    var context = canvas.getContext("2d");
context.fillStyle = "red";
context.fillRect(0, 0, 200, 100);""")],
    IS_Dynamic, IS_Public, IS_Slow,
    [ ParamDoc( "mode", "Context mode: '2d'|'webgl'", "string", NO_Default, IS_Obligated) ],
    ReturnDoc( "The context or null", "CanvasRenderingContext2D|WebGLRenderingContext", nullable=True )
)

FunctionDoc( "Canvas.setContext", "Sets the canvas context.",
    SeesDocs( "Canvas.getContext|Canvas.setContext|Canvas.clear" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "obj", "Object", "CanvasRenderingContext2D", NO_Default, IS_Obligated) ],
    NO_Returns
)

FieldDoc( "Canvas.position", """Set the coordinate model that is used for the drawing of the canvas layout.


This may be any of:

* 'absolute' Placement is relative to the top-left corner of the application window.
* 'fixed' Placement is relative to the parent canvas but not affected by the scroll position of the parent.
* 'inline' Placement is relative to the previous sibling. The canvas will be placed to the right of the previous sibling.
* 'inline-break' Same as inline, but if the canvas does not fit inside his parent, the new canvas will be pushed bellow.
* 'relative' Placement is relative to the parent-canvas.
""",
    NO_Sees,
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);

ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);

canvas.left = 100; // 100px relative to window.canvas
canvas.top = 50; // 50px relative to window.canvas

// at this stage, canvas is now positioned at (100,50) relative to its parent (window.canvas)

canvas.position = "absolute";

// at this stage, canvas is now positioned at (100,50) relative to the application window""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "string",
    "relative"
)

FieldDoc( "Canvas.width", """Get or set the width of the Canvas.",

The operation is slow when the canvas' context is already instantiated with 'Canvas.getContext', because it needs to restructure several things internally.
If the canvas' context did not have a 'Canvas.getContext' call yet, it is a fast method.""",
    SeesDocs( "Canvas.minWidth|Canvas.width|Canvas.maxWidth" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100);
canvas.width = 150;""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.minWidth", "Get or set the minimal width of the Canvas.",
    SeesDocs( "Canvas.minWidth|Canvas.width|Canvas.maxWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.maxWidth", "Get or set the maximal width of the Canvas.",
    SeesDocs( "Canvas.minWidth|Canvas.width|Canvas.maxWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.height", """Get or set the height of the Canvas.

The operation is slow when the canvas' context is already instantiated with 'Canvas.getContext', because it needs to restructure several things internally.
If the canvas' context did not have a 'Canvas.getContext' call yet, it is a fast method.""",
    SeesDocs( "Canvas.minHeight|Canvas.height|Canvas.maxHeight" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100);
canvas.height = 150;""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.minHeight", "Get or set the minimal height of the Canvas.",
    SeesDocs( "Canvas.minHeight|Canvas.height|Canvas.maxHeight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.maxHeight", "Get or set the maximum height of the Canvas.",
    SeesDocs( "Canvas.minHeight|Canvas.height|Canvas.maxHeight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.left", """Get or set the position of the top-left corner of the canvas.

The behavior depends on the value of the 'Canvas.position' property.""",
    SeesDocs( "Canvas.position|Canvas.left|Canvas.right|Canvas.marginLeft|Canvas.marginRight|Canvas.staticLeft|Canvas.staticRight" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.left = 100;"""
)],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.right", """Get or set the position of the top-right corner of the canvas.

The behavior depends on the value of the `Canvas.position` property.""",
    SeesDocs( "Canvas.left|Canvas.right|Canvas.marginLeft|Canvas.marginRight|Canvas.staticLeft|Canvas.staticRight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.top", """Get or set the top position of the top-left corner of the canvas.

The behavior depends on the value of the `Canvas.position` property.""",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginTop|Canvas.marginBottom|Canvas.staticTop|Canvas.staticBottom" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas);
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.top = 100;
""") ],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.bottom", """Get or set the bottom position of the canvas.

The behavior depends on the value of the `Canvas.position` property.""",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginTop|Canvas.marginBottom|Canvas.staticTop|Canvas.staticBottom" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.scrollLeft", """Get or set the horizontal scroll position of the canvas.

The offset is applied to the left property of all with a `fixed` or `absolute` position.

This property is useful to implement mouse scrolling or to move multiple canvases at once.

The value can't be negative unless the property 'Canvas.allowNegativeScroll' is set to 'true'.""",
    SeesDocs( "Canvas.scrollTop|Canvas.scrollLeft|Canvas.scrollBottom|Canvas.allowNegativeScroll" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas)
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.left = 100;
document.canvas.offsetLeft = 10;""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.scrollTop", """Get or set the vertical scroll position of the canvas.",

The offset is applied to the left property of all children.
Children with a 'fixed' 'Canvas.position' or an 'absolute' 'Canvas.position' are not impacted.

This property is useful to implement mouse scrolling or to move multiple canvases at once.

The value can't be negative unless the property 'Canvas.allowNegativeScroll' is set to 'true'.  """,
    SeesDocs( "Canvas.scrollTop|Canvas.scrollLeft|Canvas.scrollBottom|Canvas.allowNegativeScroll" ),
    [ExampleDoc("""var canvas = new Canvas(200, 400);
var ctx = canvas.getContext("2d");
document.canvas.add(canvas)
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);
canvas.top = 100;
document.canvas.offsetTop = 10;""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.allowNegativeScroll", "Get or set if negative scrolling is allowed.",
    SeesDocs( "Canvas.scrollTop|Canvas.scrollLeft|Canvas.scrollBottom|Canvas.allowNegativeScroll" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.visible", "Get or set the visibility of a canvas.",
    SeesDocs( "Canvas.visible|Canvas.opacity|Canvas.overflow|Canvas.coating" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.overflow", """Get or set the overflow of a canvas.

This property defines whether children will be clipped inside the canvas boundaries or not.""",
    SeesDocs( "Canvas.visible|Canvas.opacity|Canvas.overflow|Canvas.coating" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100);
document.canvas.add(canvas);
var canvas2 = new Canvas(50, 50);
canvas.add(canvas2);
canvas2.left = 50;
canvas2.top = -10;
canvas.overflow = false; """)],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc("Canvas.scrollable", "Enable or disable scrolling on a canvas.",
    SeesDocs("Canvas.scrollableX|Canvas.scrollableY"),
    [ExampleDoc("""var c = new Canvas(200, 400);
c.overflow = false;
c.scrollableY = true;

var c2 = new Canvas(200, 1000);
c2.scrollable = false;

var ctx = c2.getContext("2d");
var grd = ctx.createLinearGradient(0,0,0,1000);
grd.addColorStop(0,"blue");
grd.addColorStop(1,"red");

ctx.fillStyle = grd;
ctx.fillRect(0, 0, 200, 1000);

c.add(c2);
document.canvas.add(c);""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    "false"
)

FieldDoc("Canvas.scrollableX", "Enable or disable scrolling on X axis for a canvas.",
    SeesDocs("Canvas.scrollable|Canvas.scrollableY"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    "false"
)

FieldDoc("Canvas.scrollableY", "Enable or disable scrolling on Y axis for a canvas.",
    SeesDocs("Canvas.scrollable|Canvas.scrollableX"),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    "false"
)

FieldDoc( "Canvas.coating", """Get or set the coating of a canvas.

Coating is always surrounding the canvas and does not change its size.
Setting a coating greater than 0 allows you to draw at negative coordinates, beyond the bounding rectangle of the canvas.""",
    SeesDocs( "Canvas.visible|Canvas.opacity|Canvas.overflow|Canvas.coating" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100);
canvas.coating = 100;
canvas.left = 300;
canvas.top = 300;

var ctx = canvas.getContext("2d");
ctx.fillStyle = "red";
ctx.fillRect(0, 0, canvas.width, canvas.height);

ctx.strokeStyle = "blue";
ctx.strokeRect(-100, -100, canvas.width+200, canvas.height+200);

document.canvas.add(canvas);""")],
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.opacity", """Get or set the opacity of a canvas.

The children of the canvas are affected as well.
The value must be between '0.0' and '1.0'.""",
    SeesDocs( "Canvas.visible|Canvas.opacity|Canvas.overflow|Canvas.coating" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.staticLeft", "Get or set the static-left flag of the canvas.",
    SeesDocs( "Canvas.left|Canvas.right|Canvas.marginLeft|Canvas.marginRight|Canvas.staticLeft|Canvas.staticRight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.staticRight", "Get or set the static-right flag of the canvas.",
    SeesDocs( "Canvas.left|Canvas.right|Canvas.marginLeft|Canvas.marginRight|Canvas.staticLeft|Canvas.staticRight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.staticTop", "Get or set the static-right flag of the canvas.",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginTop|Canvas.marginBottom|Canvas.staticBottom" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.staticBottom", "Get or set the static-top flag of the canvas.",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginTop|Canvas.marginBottom|Canvas.staticTop" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.fluidHeight", "Get or set the fluid-height flag of the canvas.",
    SeesDocs( "Canvas.fluidHeight|Canvas.fluidWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.fluidWidth", "Get or set the fluid-width flag of the canvas.",
    SeesDocs( "Canvas.fluidHeight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.id", "Get or set the id for this canvas.",
    NO_Sees,
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "string",
    NO_Default
)

FieldDoc( "Canvas.marginLeft", "Get or set the left margin for the canvas.",
    SeesDocs( "Canvas.left|Canvas.right|Canvas.marginRight|Canvas.staticLeft|Canvas.staticRight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.marginRight", "Get or set the right margin for the canvas.",
    SeesDocs( "Canvas.left|Canvas.right|Canvas.marginLeft|Canvas.staticLeft|Canvas.staticRight" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "float",
    NO_Default
)

FieldDoc( "Canvas.marginTop", "Get or set the top margin for the canvas.",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginBottom|Canvas.staticTop|Canvas.staticBottom" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.marginBottom", "Get or set the bottom margin for the canvas.",
    SeesDocs( "Canvas.top|Canvas.bottom|Canvas.marginTop|Canvas.staticTop|Canvas.staticBottom" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_ReadWrite,
    "boolean",
    NO_Default
)

FunctionDoc( "Canvas.cursor", """Set the cursor for this canvas.

The cursortype may be any of "default"|"arrow"|"beam"|"text"|"pointer"|"grabbing"|"drag"|"hidden"|"none"|"col-resize".""",
    NO_Sees,
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Fast,
    [ ParamDoc( "type", "Cursor name", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FieldDoc( "Canvas.clientWidth", """Get the clientWidth of this canvas.

This is the physical width of a canvas including the left and right coating on each side.""",
    SeesDocs( "Canvas.clientHeight|Canvas.clientTop|Canvas.clientLeft" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100); // create a 100x100 canvas
canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
console.log(canvas.width) // ---> 100
console.log(canvas.clientWidth) // ---> 160 (30 coating left + 100 width + 30 coating right = 160)
""")],
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.clientHeight", """Get the clientHeight of this canvas.

This is the physical height of a canvas including the top and bottom coating.""",
    SeesDocs( "Canvas.clientWidth|Canvas.clientTop|Canvas.clientLeft" ),
    [ExampleDoc("""var canvas = new Canvas(100, 100); // create a 100x100 canvas
canvas.coating = 30; // add a coating (padding) of 30 outside the canvas
console.log(canvas.height) // ---> 100
console.log(canvas.clientHeight) // ---> 160 (30 coating top + 100 height + 30 coating bottom = 160)""")],
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.clientLeft", "Get the clientLeft of this canvas.",
    SeesDocs( "Canvas.clientHeight|Canvas.clientWidth|Canvas.clientTop|Canvas.clientLeft" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.clientTop", "Get the clientTop of this canvas.",
    SeesDocs( "Canvas.clientHeight|Canvas.clientWidth|Canvas.clientTop|Canvas.clientLeft" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.__visible", "Findout if the canvas will be displayed.",
    SeesDocs( "Canvas.__fixed|Canvas.__outofbound|Canvas.__outofbound" ),
    NO_Examples,
    IS_Dynamic, IS_Private, IS_Readonly,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.contentWidth", "Get the contentWidth of this canvas.",
    SeesDocs( "Canvas.innerHeight|Canvas.innerWidth|Canvas.contentHeight|Canvas.contentWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.contentHeight", "Get the contentHeight of this canvas.",
    SeesDocs( "Canvas.innerHeight|Canvas.innerWidth|Canvas.contentHeight|Canvas.contentWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.innerWidth", "Get the innerWidth of this canvas.",
    SeesDocs( "Canvas.innerHeight|Canvas.innerWidth|Canvas.contentHeight|Canvas.contentWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.innerHeight", "Get the innerHeight of this canvas.",
    SeesDocs( "Canvas.innerHeight|Canvas.innerWidth|Canvas.contentHeight|Canvas.contentWidth" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "integer",
    NO_Default
)

FieldDoc( "Canvas.__fixed", "Findout if there is a fixed ancestor.",
    SeesDocs( "Canvas.__fixed|Canvas.__outofbound|Canvas.__outofbound" ),
    NO_Examples,
    IS_Dynamic, IS_Private, IS_Readonly,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.__top", "Get the calculated value for the top property of the canvas.",
    SeesDocs( "Canvas.__top|Canvas.__left" ),
    NO_Examples,
    IS_Dynamic, IS_Private, IS_Readonly,
    "float",
    NO_Default
)

FieldDoc( "Canvas.__left", "Get the calculated value for the left property of the canvas.",
    SeesDocs( "Canvas.__top|Canvas.__left" ),
    NO_Examples,
    IS_Dynamic, IS_Private, IS_Readonly,
    "float",
    NO_Default
)

FieldDoc( "Canvas.ctx", "Get the context for the canvas.",
    SeesDocs( "Canvas.ctx" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "Canvas",
    NO_Default
)

FieldDoc( "Canvas.__outofbound", "Findout if the canvas is out of bound.",
    SeesDocs( "Canvas.__fixed|Canvas.__outofbound|Canvas.__visible" ),
    NO_Examples,
    IS_Dynamic, IS_Private, IS_Readonly,
    "boolean",
    NO_Default
)

FieldDoc( "Canvas.inherrit", "Object instance of CanvasInherrit class.",
    SeesDocs( "Canvas|CanvasInherrit" ),
    NO_Examples,
    IS_Dynamic, IS_Public, IS_Readonly,
    "CanvasInherrit",
    NO_Default
)

ConstructorDoc( "Canvas", """Constructs a canvas instance.

The new canvas is orphaned and doesn't belong to the view hierarchy until added as a child to the root hierarchy.
A canvas will not be displayed until it is a descendent of the root canvas (`document.canvas`).""",
    SeesDocs( "CanvasInherrits|document.canvas" ),
    [ExampleDoc("""// Create a new 200x100 canvas (logical pixels)
var myCanvas = new Canvas(200, 100);
// At this stage, myCanvas is an orphaned canvas.
// Now we add myCanvas to the root canvas (with the add() method)
document.canvas.add(myCanvas);
// At this stage, myCanvas is "rooted" and document.canvas becomes its parent.
""")],
    [
        ParamDoc( "width", "Width size", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "height", "Height size", "integer", NO_Default, IS_Obligated ),
        ParamDoc( "options", "Options object", ObjectDoc([("lazy", "lazy evaluation", "boolean")]), NO_Default, IS_Optional )
    ],
    ReturnDoc( "Canvas instance", "Canvas" )
)

EventDoc("Canvas.load",
    "Event fired when the canvas will be draw for the first time.",
    params=[ParamDoc("event", "Event object", ObjectDoc([]))]
)

EventDoc("Canvas.resize",
    "Event fired after a canvas has been resized",
    params=[ParamDoc("event", "Event object", ObjectDoc([]))]
)

EventDoc("Canvas.change", "Callback for change events.\n\nChange event are fired when the size of the canvas is updated",
    SeesDocs( "Canvas.onresize|Canvas.onload|Canvas.change" ),
    NO_Examples,
    [
        ParamDoc("obj", "event object", ObjectDoc([
            ("property", "Name of the property being changed", "string"),
            ("value", "Value of the property being changed", "string|float|integer|boolean")
        ]))
    ]
)

baseMouseEventObject = [
    ("x",       "Horizontal cursor position", "integer"),
    ("y",       "Vertical cursor position", "integer"),
    ("clientX", "??", "integer"),
    ("clientY", "??", "integer"),
    ("layerX",  "Horizontal cursor position relative to the canvas", "integer"),
    ("layerY",  "Vertical cursor position relative to the canvas", "integer"),
    ("target",  "`Canvas` where the event is taking place", "Canvas")
]

mouseEventPosition = list(baseMouseEventObject)
mouseEventPosition.extend([
    ("xrel", "Relative value of `x` compared to the previous event", "integer"),
    ("yrel", "Relative value of `y` compared to the previous event", "integer")
])
mouseEventPosition = [ParamDoc("event", "Event object", ObjectDoc(mouseEventPosition))]


mouseEventClick = list(baseMouseEventObject)
mouseEventClick.extend([
    ("which", "ID of the button pressed", "integer")
])
mouseEventClick = [ParamDoc("event", "Event object", ObjectDoc(mouseEventClick))]


mouseEventDrag = list(baseMouseEventObject)
mouseEventDrag.extend([
    ("source", "Source canvas for this event", "Canvas")
])
mouseEventDrag = [ParamDoc("event", "Event object", ObjectDoc(mouseEventDrag))]


mouseEventDragTarget = list(baseMouseEventObject)
mouseEventDragTarget.extend([
    ("source", "Source canvas for this event", "Canvas"),
    ("target", "Target canvas for this event", "Canvas")
])
mouseEventDragTarget = [ParamDoc("event", "Event object", ObjectDoc(mouseEventDragTarget))]


EventDoc("Canvas.mousemove",
    "Event fired when the mouse pointer is moving over the canvas",
    params=mouseEventPosition
)

EventDoc("Canvas.mousedown",
    "Event fired when the mouse pointer is over the element, and the mouse button is pressed.",
    params=[ParamDoc("event", "Event object", ObjectDoc(baseMouseEventObject))]
)

EventDoc("Canvas.mouseup",
    "Event fired when the mouse pointer is over the element, and the mouse button has been pressed and then released.",
    params=[ParamDoc("event", "Event object", ObjectDoc(baseMouseEventObject))]
)

EventDoc("Canvas.dblclick",
    "Event fired when the mouse pointer is over the element, and the mouse button has been pressed twice.",
    params=mouseEventClick
)

EventDoc("Canvas.dragstart",
    "Event fired when the user starts to drag a canvas",
    params=mouseEventDrag
)

EventDoc("Canvas.dragend", "Event fired when the user has finished dragging a canvas",
    params=mouseEventDrag
)

EventDoc("Canvas.dragover",
    "Event fired when a canvas is being dragged over a target canvas.\n This event is fired on the targeted canvas.",
    params=mouseEventDragTarget
)

EventDoc("Canvas.drop",
    "Event fired when a canvas has been dragged over a target canvas.\n This event is fired on the targeted canvas.",
    params=mouseEventDragTarget
)

EventDoc("Canvas.mousewheel",
    "Event fired when the mouse wheel is activated onto a canvas.",
    params=[ParamDoc("event", "Event object", ObjectDoc(baseMouseEventObject))]
)
