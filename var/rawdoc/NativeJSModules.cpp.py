from dokumentor import *

NamespaceDoc( "Exports", "Exposes modules",
	SeesDocs( "Exports|Module" ),
	NO_Examples
)

NamespaceDoc( "Module", "Module that is exported",
	SeesDocs( "Exports|Module" ),
	NO_Examples
)

FieldDoc( "global.modules", "Exposed Modules",
	SeesDocs( "Exports|Module" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	"object",
	NO_Default
)

FieldDoc( "Module.exports", "Exported Modules",
	SeesDocs( "Exports|Module" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	"object",
	NO_Default
)

FieldDoc( "Module.id", "An internal id",
	SeesDocs( "Exports|Module|Module.id|Module.name" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	"integer",
	NO_Default
)

FieldDoc( "Module.name", "The module.name",
	SeesDocs( "Exports|Module|Module.id|Module.name" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Readonly,
	"string",
	NO_Default
)

FunctionDoc( "global.require", "Asserts that a module must be loaded",
	SeesDocs( "Module|Exports|Module.name" ),
	NO_Examples,
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "path", "modulename", "string", NO_Default, IS_Obligated ) ],
	ReturnDoc( "modulename", "string" )
) 
