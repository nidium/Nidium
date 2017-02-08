# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "Window", """Window class.

It is not possible to create an instance of this class. 'global.window' is already available.""",
    SeesDocs( "global|global.window|Window" ),
    NO_Examples,
    products=["Frontend"]
)

NamespaceDoc( "MouseEvent", "Class that describes mouse events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples
)

NamespaceDoc( "MouseDrag", "Class that describes drag events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples
)

NamespaceDoc( "WindowEvent", "Class that describes window events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples,
    section="Window"
)

NamespaceDoc( "TextInputEvent", "Class that describes textinput events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples
)

NamespaceDoc( "keyEvent", "Class that describes key events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples
)

NamespaceDoc( "NMLEvent", "Class that describes events.",
    SeesDocs( "global.window|NMLEvent|Window|WindowEvent|keyEvent|TextInputEvent|MouseEvent|DragEvent" ),
    NO_Examples
)

EventDoc( "Window._onready", "Function that is called when the window is ready.",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", "WindowEvent", NO_Default, IS_Obligated ) ]
)

EventDoc( "Window._onclose", "Function that is called when the window will be closed.",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", "WindowEvent", NO_Default, IS_Obligated ) ]
)

EventDoc( "Window._onassetready", """Function that is called when the window has loaded all the assets.""",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", ObjectDoc([("data", "string in utf8 encoding", "string"),
                                                    ("tag", "tag name", "string"),
                                                    ("id", "id of the object", "string")]), NO_Default, IS_Obligated ) ]
)

EventDoc( "Window._onfocus", "Function that is called when the window receives the focus.",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    NO_Params
)

EventDoc( "Window._onblur", "Function that is called when the window looses the focus.",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    NO_Params
)

EventDoc( "Window._onmousewheel", """Function that is called when the window gets an mouse wheel event.""",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", ObjectDoc([("xrel", "x relative position", "integer"),
                                                    ("yres", "x relative position", "integer"),
                                                    ("x_pos", "x position", "integer"),
                                                    ("y_pos", "y position", "integer")
                                                    ]), NO_Default, IS_Obligated ) ]
)

for i in ["_onkeydown", "_onkeyup" ]:
    EventDoc( "Window." + i, """Function that is called when the window gets an  " + i + " event.""",
        SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
        NO_Examples,
        [ ParamDoc( "event", "EventMessage", ObjectDoc([ ("keyCode", "keycode used", "integer"),
                                                         ("location", "location used", "integer"),
                                                         ("altKey", "alt used", "boolean"),
                                                         ("ctrlKey", "ctrl used", "boolean"),
                                                         ("shiftKey", "shift used", "boolean"),
                                                         ("metaKey", "meta used", "boolean"),
                                                         ("spaceKey", "space used", "boolean"),
                                                         ("repeat", "repeating", "boolean") ]), NO_Default, IS_Obligated ) ]
    )

EventDoc( "Window._ontextinput", """function that is called when the window gets an  " + i + " event.""",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", ObjectDoc([("val", "value (utf8)", "string")]), NO_Default, IS_Obligated ) ]
)

EventDoc( "Window._onsystemtrayclick", """Function that is called when the window gets an  " + i + " event.""",
    SeesDocs( "Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", ObjectDoc([("id", "event id", "integer")]), NO_Default, IS_Obligated ) ]
)

for i in ["_onmousedown", "_onmouseup" ]:
    EventDoc( "Window." + i, """Function that is called when the window gets an  " + i + " event.""",
        SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
        NO_Examples,
        [ ParamDoc( "event", "EventMessage", ObjectDoc([ ("x_pos", "x position", "integer"),
                                                        ("y_pos", "y position", "integer"),
                                                        ("clientX", "client x position", "integer"),
                                                        ("clientY", "client y position", "integer"),
                                                        ("which", "mouse button", "integer") ]), NO_Default, IS_Obligated ) ]
    )

for i in ["_onFileDragEnter", "_onFileDragLeave", "_onFileDrag", "_onFileDragDrop" ]:
    EventDoc( "Window." + i, """function that is called when the window gets an  " + i + " event.  """,
        SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
        NO_Examples,
        [ ParamDoc( "event", "EventMessage", ObjectDoc([ ("x_pos", "x position", "integer"),
                                                        ("y_pos", "y position", "integer"),
                                                        ("clientX", "client x position", "integer"),
                                                        ("clientY", "client y position", "integer"),
                                                        ("files", "Array of filenames", "[string]") ]), NO_Default, IS_Obligated ) ]
    )

EventDoc( "Window._onmousemove", """function that is called when the window gets an  mousemovement event.""",
    SeesDocs( "global.window|Window|Window._onassetready|Window._onready|Window._onbeforeclose|Window._onfocus|Window._onblur|Window._onmousewheel|Window._onkeydown|Window._onkeyup|Window._ontextinput|Window._onsystemtrayclick|Window._onmousedown|Window._onmouseup|Window._onFileDragEnter|Window.onFileDragLeave|Window._onFileDrag|Window._onFileDrop|Window._onmousemove"),
    NO_Examples,
    [ ParamDoc( "event", "EventMessage", ObjectDoc([ ("x_pos", "x position", "integer"),
                                                        ("y_pos", "y position", "integer"),
                                                        ("xrel", "relative x position", "integer"),
                                                        ("yrel", "relative y position", "integer"),
                                                        ("clientX", "client x position", "integer"),
                                                        ("clientY", "client y position", "integer"),
                                                        ("files", "Array of filenames", "[string]") ]), NO_Default, IS_Obligated ) ]
)

topics = {"left": "integer", "top": "integer", "innerWidth": "integer", "innerHeight": "integer", "outerHeight": "integer", "title": "string" }
for i,typed in topics.items():
    FieldDoc( "Window." + i, "Set/Get the windows's " + i + " value.",
        SeesDocs( "'" + "|".join( topics.keys() )  + "'" ),
        NO_Examples,
        IS_Static, IS_Public, IS_ReadWrite,
        typed,
        NO_Default
    )

topics = { "cursor": "string", "titleBarColor": "integer", "titleBarControlOffsetX": "integer", "titleBarControlOffsetY": "integer" }
for i,typed in topics.items() :
    expl = ""
    if i == 'cursor':
        expl = "\nAllowed values are 'default'|'arrow'|'beam'|'text'|'pointer'|'grabbing'|'drag'|'hidden'|'none'|'col-resize'"
    FieldDoc( "Window." + i, "Set/Get the windows's " + i + " value.",
        SeesDocs( "'" +"|".join( topics.keys() )  + "'" ),
        NO_Examples,
        IS_Static, IS_Public, IS_ReadWrite,
        typed,
        NO_Default
    )

FunctionDoc( "Window.setSize", "Set the size of the window.",
    SeesDocs( "Window.center|Window.setPosition|Window.setSize" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "width", "The width to set to", "integer", NO_Default, IS_Obligated ),
    ParamDoc( "heigth", "The heigth to set to", "integer", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Window.openURL", "Open an url in a new browser session.",
    SeesDocs( "global.window|Window|Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    [ParamDoc( "url", "The url to open", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Window.exec", "Executes a command in the background.",
    SeesDocs( "global.window|Window|Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    [ParamDoc( "cmd", "The cmd with its arguments", "string", NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Window.openDirDialog", "Opens a directory selection dialog.",
    SeesDocs( "global.window|Window|Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    [CallbackDoc( "fn", "The callback function", [ParamDoc( "list", "List of the selected items", "[string]", NO_Default, IS_Obligated ) ] ) ],
    NO_Returns
)

FunctionDoc( "Window.openFileDialog", "Opens a file selection dialog.",
    SeesDocs( "global.window|Window|Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    [ParamDoc( "types", "list of allowed extensions or null", "[string]", NO_Default, IS_Obligated ),
      CallbackDoc( "fn", "The callback function", [ParamDoc( "list", "List of the selected items", "[string]", NO_Default, IS_Obligated ) ] ) ],
    NO_Returns
)

FunctionDoc( "Window.requestAnimationFrame", "Execute a callback for the next frame.",
    SeesDocs( "global.window|Window|Window.requestAnimationFrame|Window.setFrame" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ CallbackDoc( "fn", "The callback function", [ParamDoc( "list", "List of the selected items", "[string]", NO_Default, IS_Obligated ) ] ) ],
    NO_Returns
)

FunctionDoc( "Window.center", "Positions the window in the center.",
    SeesDocs( "global.window|Window|Window.center|Window.setPosition|Window.setSize" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Window.center", "Positions the window in the center.",
    SeesDocs( "global.window|Window|Window.center|Window.setPosition|Window.setSize" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "x_pos", "X position", "integer", 0, IS_Obligated ),
     ParamDoc( "y_pos", "Y position", "integer", 0, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Window.notify", "Send a notification to the systemtray.",
    SeesDocs( "global.window|Window|Window.notify|Window.setSystemTray" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "title", "Title text", "string", NO_Default, IS_Obligated ),
     ParamDoc( "body", "Body text", "string", NO_Default, IS_Obligated ),
     ParamDoc( "sound", "Play sound", "boolean", 'false', IS_Optional ) ],
    NO_Returns
)

FunctionDoc( "Window.quit", "Quits this window instance.",
    SeesDocs( "global.window|Window|Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Window.close", "Closes this window instance.",
    SeesDocs( "Window.exec|Window.openURL|Window.open|Window.close|Window.quit" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Slow,
    NO_Params,
    NO_Returns
)

FunctionDoc( "Window.notify", "Send a notification to the systemtray.",
    SeesDocs( "global.window|Window|Window.notify|Window.setSystemTray" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ParamDoc( "config", "Systemtray", ObjectDoc([("icon", "Icon name", 'string'),
                                                    ("menu", "array of objects", ObjectDoc([('id', "identifier", "integer"),
                                                                                            ("title", "name", "string")], IS_Array))
                                                        ]), NO_Default, IS_Obligated ) ],
    NO_Returns
)

FunctionDoc( "Window.setFrame", "Execute a callback for the next frame.",
    SeesDocs( "global.window|Window|Window.requestAnimationFrame|Window.setFrame" ),
    NO_Examples,
    IS_Static, IS_Public, IS_Fast,
    [ ParamDoc( "x_pos", "x position", "integer|string", '0|center', IS_Obligated ),
      ParamDoc( "y_pos", "x position", "integer|string", '0|center', IS_Obligated ),
      ParamDoc( "hh", "? position", "integer", '0', IS_Obligated ),
      ParamDoc( "nn", "? position", "integer", '0', IS_Obligated ) ],
    NO_Returns
)

FieldDoc( "Window.canvas", "The main canvas instance.",
    SeesDocs( "global.window|Window|Canvas" ),
    NO_Examples,
    IS_Static, IS_Public, IS_ReadWrite,
    'Canvas',
    NO_Default
)

NamespaceDoc( "Navigator", "Navigator/browser description.",
    SeesDocs("global.window|Window|Navigator|Window.navigator|Window.__nidium__"),
    [ExampleDoc("console.log(JSON.stringify(window.navigator));")]
)

navigator = {"language": ('string', "The systems language. Currently fixed on 'us-en'."),
            "vibrate": ('boolean', "Can this device vibrate (Currently fixed on 'false')."),
            "appName": ('string', "The browser name (currently fixed on 'nidium')."),
            "appVersion": ('string', "The browsers build version."),
            "platform": ('string', "The systems version ('Win32'|'iPhone Simulator'|'iPhone'|'Macintosh'|'Mac'|'MacOSX'|'FreeBSD'|'DragonFly'|'Linux'|'Unix'|'Posix'|'Unknown')."),
            "userAgent": ('string', 'The browser identification string.')
        }
for i, details in navigator.items():
    FieldDoc("Navigator." + i, "Details for this browser concerning: " + i + "\n" + details[1],
        SeesDocs("global.window|Window|Navigator|Window.navigator"),
        [ExampleDoc("console.log('" + i + " is now: ' +  window.navigator." + i + ");")],
        IS_Static, IS_Public, IS_Readonly,
        details[0],
        NO_Default
    )

FieldDoc("Window.navigator", "Details for this browser.",
    SeesDocs("global.window|Window|Navigator|Window.navigator|global.window"),
    [ExampleDoc("console.log(JSON.stringify(window.navigator));")],
    IS_Static, IS_Public, IS_Readonly,
    "Navigator",
    NO_Default
)

FieldDoc("Window.__nidium__", "Details for this browser's framework.",
    SeesDocs("global.window|Window|Navigator|Window.navigator|Window.__nidium__|global.window"),
    [ExampleDoc("console.log(JSON.stringify(window.navigator));")],
    IS_Static, IS_Public, IS_Readonly,
    ObjectDoc([("version", "The version of nidium", "string"),
                ("build", "The build identifier", "string"),
                ("revision", "The revision identifier", "string")]),
    NO_Default
)

