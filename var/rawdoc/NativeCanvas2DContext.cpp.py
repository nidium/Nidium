from dokumentor import *

NamespaceDoc( "ImageData", "ImageData Class",
	SeesDocs( "Image|Canvas" ),
	NO_Examples
)

NamespaceDoc( "CanvasRenderingContext2D", "Class to render a 2d canvas",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasPattern|CanvasRenderingContext2D" ),
	NO_Examples
)

NamespaceDoc( "CanvasGradient", "Canvas gradient Class",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasPattern|CanvasRenderingContext2D" ),
	NO_Examples
)

NamespaceDoc( "CanvasGLProgram", "Canvas GL program Class",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasPattern|CanvasRenderingContext2D" ),
	NO_Examples
)

NamespaceDoc( "CanvasPattern", "Canvas pattern Class",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasPattern|CanvasRenderingContext2D" ),
	NO_Examples
)

FunctionDoc( "CanvasRenderingContext2D.breakText", "Wrap text over multiple lines based on a maximum width",
	SeesDocs( "CanvasRenderingContext2D.breakText|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fillText|CanvasRenderingContext2D.strokeText|CanvasRenderingContext2D.measureText" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "text", "Text string", "string", NO_Default, IS_Obligated ),
	ParamDoc( "maxWidth", "The maximal width where the text should fit in", "float", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Object with keys: height/double, lines/[string]", "Object" )
)

FunctionDoc( "CanvasRenderingContext2D.fillText", "Put text in a textbox.",
	SeesDocs( "CanvasRenderingContext2D.breakText|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fillText|CanvasRenderingContext2D.strokeText|CanvasRenderingContext2D.measureText" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "text", "Text string", "string", NO_Default, IS_Obligated ),
	ParamDoc( "x", "X position", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "y", "Y position", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "maxWidth", "Maximum width where the text must fit in.", "integer", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.strokeText", "Stroke text",
	SeesDocs( "CanvasRenderingContext2D.breakText|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fillText|CanvasRenderingContext2D.strokeText|CanvasRenderingContext2D.measureText" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "text", "Text string", "string", NO_Default, IS_Obligated ),
	ParamDoc( "x", "X position", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "y", "Y position", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "maxWidth", "Maximum width where the text must fit in.", "integer", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.shadow", "Put a shadow around on object.",
	SeesDocs( "CanvasRenderingContext2D.shadow" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.beginPath", "Begin a path.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.moveTo", "Set the next coordinate on a path.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.lineTo", "Set the next coordinate on a line.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.fill", "Fill an object.",
	SeesDocs( "CanvasRenderingContext2D.fill|CanvasRenderingContext2D.stroke|CanvasRenderingContext2D.clip" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.stroke", "Stroke an object.",
	SeesDocs( "CanvasRenderingContext2D.fill|CanvasRenderingContext2D.stroke|CanvasRenderingContext2D.clip" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.closePath", "Close the path.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.clip", "Clip an object.",
	SeesDocs( "CanvasRenderingContext2D.fill|CanvasRenderingContext2D.stroke|CanvasRenderingContext2D.clip" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.rect", "Draw a rectangle.",
	SeesDocs( "CanvasRenderingContext2D.rect|CanvasRenderingContext2D.arc|CanvasRenderingContext.arcTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x1", "X1 position", "integer", NO_Default, IS_Obligated ), 
	 ParamDoc( "y1", "Y1 position", "integer", NO_Default, IS_Obligated ), 
	 ParamDoc( "x2", "X2", "integer", NO_Default, IS_Obligated ), 
	 ParamDoc( "y2", "Y2", "integer", NO_Default, IS_Obligated ), 
	 ParamDoc( "radius", "Radius", "float", NO_Default, IS_Obligated )], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.arcTo", "Draw a arc to a positione.",
	SeesDocs( "CanvasRenderingContext2D.rect|CanvasRenderingContext2D.arc|CanvasRenderingContext.arcTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "width", "Width", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "height", "Height", "float", NO_Default, IS_Obligated ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.arc", "Draw an arc.",
	SeesDocs( "CanvasRenderingContext2D.rect|CanvasRenderingContext2D.arc|CanvasRenderingContext.arcTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "radius", "Radius", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "start", "Start Angle", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "end", "End Angle", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "ccw", "cww", "boolean", "false", IS_Optional ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.quadraticCurveTo", "Set the next coordinate for a quadratic curve on a path.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "cx", "cX position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "cy", "cY position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.bezierCurveTo", "Set the next coordinate for a bezier curve on a path.",
	SeesDocs( "CanvasRenderingContext2D.beginPath|CanvasRenderingContext2D.moveTo|CanvasRenderingContext2D.lineTo|CanvasRenderingContext2D.closePath|CanvasRenderingContext2D.quadraticCurveTo|CanvasRenderingContext2D.bezierCurveTo" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "cx1", "cX1position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "cy1", "cY1position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "cx2", "cX2position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "cy2", "cY2 position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ) ], 
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.rotate", "Rotate an object",
	SeesDocs( "CanvasRenderingContext2D.rotate|CanvasRenderingContext2D.scale|CanvasRenderingContext2D.translate|CanvasRenderingContext2D.transform" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "angle", "Angle", "float", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.scale", "Scale an object",
	SeesDocs( "CanvasRenderingContext2D.rotate|CanvasRenderingContext2D.scale|CanvasRenderingContext2D.translate|CanvasRenderingContext2D.transform" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X Position", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "y", "Y Position", "float", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.translate", "Translate an object",
	SeesDocs( "CanvasRenderingContext2D.rotate|CanvasRenderingContext2D.scale|CanvasRenderingContext2D.translate|CanvasRenderingContext2D.transform" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X Position", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "y", "Y Position", "float", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.transform", "Transfrom an object",
	SeesDocs( "CanvasRenderingContext2D.rotate|CanvasRenderingContext2D.scale|CanvasRenderingContext2D.translate|CanvasRenderingContext2D.transform|CanvasRenderingContext2D.iTransform|CanvasRenderingContext2D.setTransfrom" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "scaleX", "Scale X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewX", "Skew X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewY", "Skew Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "scaleY", "Scale Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateX", "Translate X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateY", "Translate Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "rotate", "Rotate angle", "float", NO_Default, IS_Optional ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.iTransform", "iTransfrom an object",
	SeesDocs( "CanvasRenderingContext2D.transform|CanvasRenderingContext2D.iTransform|CanvasRenderingContext2D.setTransfrom" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "scaleX", "Scale X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewX", "Skew X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewY", "Skew Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "scaleY", "Scale Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateX", "Translate X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateY", "Translate Y", "float", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.setTransform", "Set the transfrom of an object",
	SeesDocs( "CanvasRenderingContext2D.transform|CanvasRenderingContext2D.iTransform|CanvasRenderingContext2D.setTransfrom" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "scaleX", "Scale X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewX", "Skew X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "skewY", "Skew Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "scaleY", "Scale Y", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateX", "Translate X", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "translateY", "Translate Y", "float", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.save", "Save an object",
	SeesDocs( "CanvasRenderingContext2D.save|CanvasRenderingContext2D.restore" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.restore", "Restore an object",
	SeesDocs( "CanvasRenderingContext2D.save|CanvasRenderingContext2D.restore" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.createLinearGradient", "Create a linear gradient.",
	SeesDocs( "CanvasRenderingContext2D.createLinearGradient|CanvasRenderingContext2D.createRadialGradient|CanvasRenderingContext2D.createImageData|CanvasRenderingContext2D.createPattern" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x1", "X1 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "y1", "Y1 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "x2", "X2 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "y2", "Y2 value", "float", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Gradient instance", "CanvasGradient" )
)

FunctionDoc( "CanvasRenderingContext2D.getImageData", "Get an ImageData.",
	SeesDocs( "Image|ImageData.getImageData|ImageData.putImageData|ImageData.createImageData|CanvasRenderingContext2D.drawImage" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "left", "Left", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "top", "Top", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "heigth", "Heigth", "integer", NO_Default, IS_Obligated ) ],
	ReturnDoc( "ImageData instance", "ImageData" )
)

items = {"width": "integer", "height": "integer", "data": "ImageBuffer"}
for i, typed in items.items():
	FieldDoc( "ImageData." + i, "The " + i + " value.",
		SeesDocs( "ImageData.width|Image|ImageData.height|ImageData.data" ),
		NO_Examples,
		IS_Dynamic, IS_Public, IS_Readonly,
		typed,
		NO_Default
	)

FunctionDoc( "CanvasRenderingContext2D.putImageData", "Put an ImageData object.",
	SeesDocs( "Image|ImageData.getImageData|ImageData.putImageData|ImageData.createImageData|CanvasRenderingContext2D.drawImage" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "image", "Image", "ImageData", NO_Default, IS_Obligated ),
	  ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "heigth", "Heigth", "integer", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.createImageData", "Create a ImageData.",
	SeesDocs( "Image|ImageData.getImageData|ImageData.putImageData|ImageData.createImageData|CanvasRenderingContext2D.drawImage" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X Position", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "y", "Y Position", "integer", NO_Default, IS_Obligated ) ],
	ReturnDoc( "ImageData instance", "ImageData" )
)

FunctionDoc( "CanvasRenderingContext2D.createPattern", """Create a pattern.

The pattern-mode can be any of 'repeat'|'no-repeat'|'repeat-x'|'repeat-y'|'repeat-mirror'.""",
	SeesDocs( "CanvasRenderingContext2D.createLinearGradient|CanvasRenderingContext2D.createRadialGradient|CanvasRenderingContext2D.createImageData|CanvasRenderingContext2D.createPattern" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "image", "Image object", "Image", NO_Default, IS_Obligated ),
	  ParamDoc( "mode", "Pattern mode", "string", "repeat", IS_Obligated ) ],
	ReturnDoc( "CanvasPattern instance", "CanvasPattern" )
)

FunctionDoc( "CanvasRenderingContext2D.createRadialGradient", "Create a radial gradient.",
	SeesDocs( "CanvasRenderingContext2D.createLinearGradient|CanvasRenderingContext2D.createRadialGradient|CanvasRenderingContext2D.createImageData|CanvasRenderingContext2D.createPattern" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x1", "X1 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "y1", "Y1 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "r1", "R1 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "x2", "X2 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "y2", "Y2 value", "float", NO_Default, IS_Obligated ),
	  ParamDoc( "r2", "R2 value", "float", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Gradient instance", "CanvasGradient" )
)

FunctionDoc( "CanvasGradient.addColorStop", "Perform a color stop on a gradient",
	[ SeeDoc( "CanvasGradient" ) ],
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "position", "Position", "float", NO_Default, IS_Obligated ),
	ParamDoc( "color", "Color", "string", NO_Default, IS_Obligated ) ],
	NO_Returns
)

#TODO: describe the alternative parameters
FunctionDoc( "CanvasRenderingContext2D.drawImage", "Draw an Image.",
	SeesDocs( "Image|ImageData.getImageData|ImageData.putImageData|ImageData.createImageData|CanvasRenderingContext2D.drawImage" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "image", "Image object", "Image", NO_Default, IS_Obligated ),
	  ParamDoc( "x", "X Position", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "y", "Y Position", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "width", "Width", "integer", NO_Default, IS_Obligated ),
	  ParamDoc( "heigth", "Heigth", "integer", NO_Default, IS_Obligated ) ],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.measureText", "Deterimine the size of text",
	SeesDocs( "CanvasRenderingContext2D.breakText|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fillText|CanvasRenderingContext2D.strokeText|CanvasRenderingContext2D.measureText" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "text", "Text string", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "An object with keys: width/float", "Object" )
)

FunctionDoc( "CanvasRenderingContext2D.isPointPath", "Determine if the path consists out of points.",
	SeesDocs( "CanvasRenderingContext2D.isPointPath|CanvasRenderingContext2D.getPathBounds" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X position", "float", NO_Default, IS_Obligated ), 
	 ParamDoc( "y", "Y position", "float", NO_Default, IS_Obligated ) ], 
	ReturnDoc( "is it a point path", "boolean" )
)

FunctionDoc( "CanvasRenderingContext2D.getPathBounds", "Get the outer bounds of a path.",
	SeesDocs( "CanvasRenderingContext2D.isPointPath|CanvasRenderingContext2D.getPathBounds" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Object with keys: left/float, right/float, top/float, bottom/float", "Object" )
)

FunctionDoc( "CanvasRenderingContext2D.detachFragmentShader", "Detach a shader.",
	SeesDocs( "CanvasRenderingContext2D.attachFragmentShader|CanvasRenderingContext2D.detachFragmentShader" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.setVertexOffset", "Set a vertexOffset.",
	SeesDocs( "CanvasRenderingContext2D.setVertexOffset" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "vertex", "Vertex", "integer", NO_Default, IS_Obligated ) ,
	 ParamDoc( "x", "X y2yPosition", "float", NO_Default, IS_Obligated ),
	 ParamDoc( "y", "Y Position", "float", NO_Default, IS_Obligated )],
	NO_Returns
)

FunctionDoc( "CanvasRenderingContext2D.attachFragmentShader", "Attach a shader.",
	SeesDocs( "CanvasRenderingContext2D.attachFragmentShader|CanvasRenderingContext2D.detachFragmentShader" ),
	NO_Examples,
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "glsl", "GL shader", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "Program instance", "CanvasGLProgram" )
)

FunctionDoc( "CanvasGLProgram.getUniformLocation", "Get the uniform location.",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasGLProgram.getUniformLocation|CanvasGLProgram.getActiveUniforms" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "location", "Location", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "location", "integer" )
)

FunctionDoc( "CanvasGLProgram.uniform1i", "Perform a uniform1i call",
	SeesDocs( "CanvasGlProgram.uniform1i|CanvasGlProgram.uniform1f|CanvasGlProgram.uniform1iv|CanvasGlProgram.uniform2iv|CanvasGlProgram.uniform3iv|CanvasGlProgram.uniform4iv|CanvasGlProgram.uniform1fv|CanvasGlProgram.uniform2fv|CanvasGlProgram.uniform3fv|CanvasGlProgram.uniform4fv" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "location", "Location", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "value", "Value", "integer", NO_Default, IS_Obligated ),
	],
	NO_Returns
)

FunctionDoc( "CanvasGLProgram.uniform1f", "Perform a uniform1f call",
	SeesDocs( "CanvasGlProgram.uniform1i|CanvasGlProgram.uniform1f|CanvasGlProgram.uniform1iv|CanvasGlProgram.uniform2iv|CanvasGlProgram.uniform3iv|CanvasGlProgram.uniform4iv|CanvasGlProgram.uniform1fv|CanvasGlProgram.uniform2fv|CanvasGlProgram.uniform3fv|CanvasGlProgram.uniform4fv" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "location", "Location", "integer", NO_Default, IS_Obligated ),
	ParamDoc( "value", "Value", "float", NO_Default, IS_Obligated ),
	],
	NO_Returns
)

items = {"i": "integer", "f": "float" }
for i, typed in items.items( ):
	params = [ParamDoc( "location", "Location", "integer", NO_Default, IS_Obligated ) ]
	for j in range( 1, 4 ):
		params.append( ParamDoc( "value" + str( j ) , "Value of " + str( j ), typed, NO_Default, IS_Obligated ) ),
		FunctionDoc( "CanvasGLProgram.uniform" + str( j ) + i, "Perform a uniform call on " + str( j ) + " " + typed + "s.",
			SeesDocs( "CanvasGlProgram.uniform1i|CanvasGlProgram.uniform1f|CanvasGlProgram.uniform1iv|CanvasGlProgram.uniform2iv|CanvasGlProgram.uniform3iv|CanvasGlProgram.uniform4iv|CanvasGlProgram.uniform1fv|CanvasGlProgram.uniform2fv|CanvasGlProgram.uniform3fv|CanvasGlProgram.uniform4fv" ),
			NO_Examples,
			IS_Static, IS_Public, IS_Fast,
			params,
			NO_Returns
		)

FunctionDoc( "CanvasGLProgram.getUniforms", "Get the uniforms",
	SeesDocs( "Canvas|CanvasGLProgram|CanvasGLProgram.getUniformLocation|CanvasGLProgram.getActiveUniforms" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Array of objects with keys: name/string, location/int", "[Object]" )
)

FunctionDoc( "CanvasGLProgram.light", "Lights",
	SeesDocs( "CanvasRenderingContext2D|CanvasRenderingContext2D.light" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ ParamDoc( "x", "X Position", "float", NO_Default, IS_Obligated  ),
	ParamDoc( "y", "Y Position", "float", NO_Default, IS_Obligated  ),
	ParamDoc( "z", "Z Position", "float", NO_Default, IS_Obligated  ) ],
	NO_Returns
)

FieldDoc( "CanvasRenderingContext2D.imageSmoothingEnabled", "Get or set the imageSmooting flag.",
	SeesDocs( "Canvas|Image|ImageData|CanvasRenderingContext2D|CanvasRenderingContext2D.imageSmoothingEnabled" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"boolean",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.shadowOffsetX", "Get or set the shadowOffsetX",
	SeesDocs( "CanvasRenderingContext2D.shadowOffsetXnvasRenderingContext2D.shadowOffsetY|CanvasRenderingContext2D.shadowBlur|CanvasRenderingContext2D.shadowColor" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.shadowOffsetY", "Get or set the shadowOffsetY",
	SeesDocs( "CanvasRenderingContext2D.shadowOffsetXnvasRenderingContext2D.shadowOffsetY|CanvasRenderingContext2D.shadowBlur|CanvasRenderingContext2D.shadowColor" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.shadowBlur", "Get or set the shadowBlur",
	SeesDocs( "CanvasRenderingContext2D.shadowOffsetXnvasRenderingContext2D.shadowOffsetY|CanvasRenderingContext2D.shadowBlur|CanvasRenderingContext2D.shadowColor" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.shadowColor", "Get or set the shadowColor",
	SeesDocs( "CanvasRenderingContext2D.shadowOffsetXnvasRenderingContext2D.shadowOffsetY|CanvasRenderingContext2D.shadowBlur|CanvasRenderingContext2D.shadowColor" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fontSize", "Get or set the fontSize",
	SeesDocs( "CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fontFamily|CanvasRenderingContext2D.fontSkew|CanvasRenderingContext2D.fontFile" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fontStyle", "Get or set the fontStyle",
	SeesDocs( "CanvasRenderingContext2D.fillStyle|CanvasRenderingContext2D.strokeStyle|CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fontFamily|CanvasRenderingContext2D.fontSkew|CanvasRenderingContext2D.fontFile" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fontSkew", "Get or set the fontSkew",
	SeesDocs( "CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fontFamily|CanvasRenderingContext2D.fontSkew|CanvasRenderingContext2D.fontFile" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.textBaseline", "Get or set the textBaseline",
	SeesDocs( "CanvasRenderingContext2D.textAlign|CanvasRenderingContext2D.textBaseline" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.textAlign", "Get or set the textAlign",
	SeesDocs( "CanvasRenderingContext2D.textAlign|CanvasRenderingContext2D.textBaseline" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fontFamily", "Get or set the fontFamily",
	SeesDocs( "CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fontFamily|CanvasRenderingContext2D.fontSkew|CanvasRenderingContext2D.fontFile" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fontFile", "Get or set the fontFile",
	SeesDocs( "CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.fontSize|CanvasRenderingContext2D.fontFamily|CanvasRenderingContext2D.fontSkew|CanvasRenderingContext2D.fontFile" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fillStyle", "Get or set the fillStyle",
	SeesDocs( "CanvasRenderingContext2D.fillStyle|CanvasRenderingContext2D.strokeStyle|CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string|CanvasPattern",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.fillStyle", "Get or set the fillStyle",
	SeesDocs( "CanvasRenderingContext2D.fillStyle|CanvasRenderingContext2D.strokeStyle|CanvasRenderingContext2D.fontStyle|CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string|CanvasGradient",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.miterLimit", "Get or set the miter limit",
	SeesDocs( "CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)


FieldDoc( "CanvasRenderingContext2D.lineWidth", "Get or set the lineWidth",
	SeesDocs( "CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.globalAlpha", "Get or set the globalAlpha",
	SeesDocs( "CanvasRenderingContext2D.globalAlpha|CanvasRenderingContext2D.globalCompositionOperation" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"float",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.globalCompositionOperation", "Get or set the globalCompositionOperation",
	SeesDocs( "CanvasRenderingContext2D.globalAlpha|CanvasRenderingContext2D.globalCompositionOperation" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.lineCap", "Get or set the lineCap",
	SeesDocs( "CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

FieldDoc( "CanvasRenderingContext2D.lineJoin", "Get or set the lineJoin",
	SeesDocs( "CanvasRenderingContext2D.lineWidth|CanvasRenderingContext2D.lineCap|CanvasRenderingContext2D.lineJoin" ),
	NO_Examples,
	IS_Static, IS_Public, IS_ReadWrite,
	"string",
	NO_Default,
)

items = [ "height", "width" ]
for i in items:
	FieldDoc( "CanvasRenderingContext2D." + i,  "Get or set the " + i + ".",
		SeesDocs( "Canvas|Image|CanvasRenderingContext2D.width|CanvasRenderingContext2D.height" ),
		NO_Examples,
		IS_Static, IS_Public, IS_ReadWrite,
		"integer",
		NO_Default,
	)
