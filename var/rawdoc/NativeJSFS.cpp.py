from dokumentor import *

NamespaceDoc( "fs", "Filesystem access", 
	[ SeeDoc( "global.pwd" ), SeeDoc( "File"  ) ], 
	[ ExampleDoc( """fs.readDir( function( err, entries ) { 
		console.log( JSON.stringify( entries ) ); } );""") ] 
)

FunctionDoc( "fs.readDir", "Read the content of a directory", 
	[ SeeDoc( "global.pwd" ), SeeDoc( "File.isDir" ), 
	 SeeDoc( "global.listDir" ), SeeDoc( "File.rmrf" ) ], 
	[ ExampleDoc( """fs.readDir( function( err, entries ) { 
		console.log( JSON.stringify( entries ) ) ; } );""") ] , 
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)
