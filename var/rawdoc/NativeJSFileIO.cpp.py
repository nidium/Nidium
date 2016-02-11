from dokumentor import *

ClassDoc( "File", "File handling class.",
	[ SeeDoc( "fs" ), SeeDoc( "global.pwd"), SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ) ],
	[ ExampleDoc( """File.read("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
    console.log( buffer );
});""" ) ],
	NO_Inherrits,
	NO_Extends,
)

FieldDoc( "File.filesize", "The size of the file in bytes.",
	[ SeeDoc( "File.filename" ) ],
	[ ExampleDoc( """File.read("nidium.js", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
	console.log( this.filename + " "+ this.filesize + " bytes" );
});""" ) ],
	IS_Static, IS_Public, IS_Readonly,
	'string',
	NO_Default
)

FieldDoc( "File.filename", "The name of the opened file.",
	[ SeeDoc( "File.filesize" ) ],
	[ ExampleDoc( """File.read("nidium.js", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
	console.log( this.filename + " "+ this.filesize + " bytes" );
});""" ) ],
	IS_Static, IS_Public, IS_Readonly,
	'string',
	NO_Default
)

ConstructorDoc( "File", "Constructor for a File object that can do operations on files.",
	[ SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ), SeeDoc( "global.pwd" ), SeeDoc( "fs" ) ],
	[ ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function(err) {
    if (!err) {
        f.write("hello", function() {});
        f.close();
    }
});""" ) ],
	[ ParamDoc( "path", "File path relative to NML", "string", NO_Default, IS_Obligated ),
	  ParamDoc( "options", "Options object (encoding)", "object", NO_Default, IS_Optional ),
	  CallbackDoc( "callback", "Read callback function", [
		ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
		ParamDoc( "buffer", "The filecontent", "string", NO_Default, IS_Obligated) ] )
	],
	ReturnDoc( "FileHandler", "File" )
)

FunctionDoc( "File.write", "Writes a 'string' or 'arraybuffer' to a file. If the file is not opened yet, it will be opened in 'a+' mode.",
	[SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ) ],
	[ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function( err ) {
            f.seek( 0, function( ) {
                f.read( 5, function( err, buffer ) {
                    console.log( buffer );
                    f.close( );
                } );
           } );
       } )
    }
});""") ],
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "buffer", "The content to write to the file", 'string|ArrayBuffer', NO_Default, IS_Obligated ),
	 CallbackDoc( "callback", "Read callback function", [
		ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
	])],
	NO_Returns
)

FunctionDoc( "File.isDir", "Determines if the file is a Directory.",
	SeesDocs( "File.isDir|File.rmrf|File.listFiles|fs|global.pwd" ),
	[ExampleDoc( """var f = new File( "/tmp/imp.red", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.rmrf( )
		}
""" ) ],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	ReturnDoc( "Is 'true' if it is a directory, 'false' if it is a file", "boolean" )
)

FunctionDoc( "File.rmrf", "Deletes the directory and it's content.",
	[SeeDoc( "File.isDir" ), SeeDoc( "File.rmrf" ), SeeDoc( "File.listFiles" ), SeeDoc( "fs" ), SeeDoc( "global.pwd" ) ],
	[ExampleDoc( """var f = new File( "/tmp/dangerous/", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.rmrf( )
		}
""" ) ],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "File.listFiles", "List the files in a directory.",
	[SeeDoc( "File.isDir" ), SeeDoc( "File.rmrf" ), SeeDoc( "File.listFiles" ), SeeDoc( "fs" ), SeeDoc( "global.pwd" ) ],
	[ExampleDoc( """var f = new File( "/tmp/", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.listFiles( function( error, entries ) {
				if ( ! err ) {
					console.log( JSON.stringify( entries ) );
			}
		} );
		}
""" ) ],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "File.read", """Reads bytes from a file. If the file was not opened yet it will be opend in 'read' mode.

'File.read' can be used with any protocol supported by nidium (e.g. 'http://', 'file://'):""",
	SeesDocs( "File.open|File.read|File.seek|File.close|global.load" ),
	[ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function(err) {
    if (!err) {
        f.write("hello", function() {});
        f.close();
    }
});"""),
ExampleDoc("""File.read("foo.txt", function(err, buffer) {
    // buffer is an array buffer containing the data
});"""),
ExampleDoc("""File.read("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
});"""),
ExampleDoc("""File.read("http://www.example.com/foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
});""") ],
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "bytes", "Bytes to read", 'integer', 0, IS_Obligated ),
	 CallbackDoc( "callback", "Read callback function", [
		ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
		ParamDoc( "buffer", "The filecontent", "string", NO_Default, IS_Obligated) ])
	],
	ReturnDoc("The file's content", 'string' )
)

FunctionDoc( "File.open", "Opens a file.",
	[ SeeDoc( "File.open" ), SeeDoc( "File.openSync" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ), SeeDoc( "File.closeSync" ), SeeDoc( "File.write" ) ],
	[ ExampleDoc( """var f = new File( "foo.txt", { encoding: "utf8" } );
f.open( "rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function () { } );
        f.close( );
    }
});
""")],
	IS_Dynamic, IS_Public, IS_Fast,
	[ ParamDoc( "mode", "The mode in which the file must be opened (e.g. rw)", "string", NO_Default, IS_Obligated ),
      CallbackDoc( "callback", "The callback function", [
		ParamDoc( "Error", "Error description", "string", NO_Default, IS_Obligated )
		] ) ],
	NO_Returns
)

FunctionDoc( "File.seek", "Moves to a certain offset from the beginning of the file.",
	[SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ) ],
	[ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function( err ) {
			f.seek( 0, function( ) {
				f.read( 5, function( err, buffer ) {
					console.log( buffer );
					f.close( );
				} );
			} );
		} )
    }
});""") ],
	IS_Dynamic, IS_Public, IS_Fast,
	[ParamDoc( "offset", "Move to this offset of the beginning of the file", 'integer', 0, IS_Obligated ),
	 CallbackDoc( "callback", "Read callback function", [
		ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
	])],
	NO_Returns
)

FunctionDoc( "File.close", "Closes an open file.",
	[SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ) ],
	[ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function( err ) {
	if ( ! err ) {
		f.close( );
	}
});"""), ExampleDoc( """var f = new File.openSync( "foo.txt", { encoding: 'utf8' } );
f.close( ); """) ],
	IS_Dynamic, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

FunctionDoc( "File.readSync", "Read a file in a synchronous way.",
	[SeeDoc( "File.read") ],
	[ExampleDoc( """var buffer = File.readSync("foo.txt");"""),
	 ExampleDoc("""var string = File.readSync("foo.txt", { encoding: "utf8" });""") ],
	IS_Static, IS_Public, IS_Slow,
	[ParamDoc( "path", "File path relative to the current NML", 'string', NO_Default, IS_Obligated ),
	 ParamDoc( "options", "Options object (encoding)", 'object', NO_Default, IS_Optional  ) ],
	ReturnDoc( "The file's content", 'string' )
)
FunctionDoc( "File.closeSync", "Close a file in a synchronous way.",
	[ SeeDoc( "File.open" ), SeeDoc( "File.openSync" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ), SeeDoc( "File.closeSync" ), SeeDoc( "File.write" ) ],
	[ ExampleDoc( """var f = new File( "foo.txt", { encoding: "utf8" } );
f.openSync( "rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function () { } );
        f.closeSync( );
    }
});
""")],
	IS_Dynamic, IS_Public, IS_Slow,
	NO_Params,
	NO_Returns
)

FunctionDoc( "File.read", "Read a file in a synchronous way.",
	[SeeDoc( "File.readSync") ],
	[ExampleDoc( """File.read("foo.txt", function(err, buffer) {
    // buffer is an array buffer containing the data
	});""" ),
	ExampleDoc( """File.read("foo.txt", {encoding: "utf8"}, function(err, buffer) {
		// buffer is an UTF8 string containing the data
	}); """ ),
	ExampleDoc( """//readFile can be used with any protocol supported by nidium (e.g. http):
	File.readFile("http://www.example.com/foo.txt", {encoding: "utf8"}, function(err, buffer) {
		// buffer is an UTF8 string containing the data
	});
	""" ) ],
	IS_Static, IS_Public, IS_Fast,
	[ParamDoc( "path", "File path relative to the current NML", 'string', NO_Default, IS_Obligated ),
	 ParamDoc( "options", "Options object (encoding)", 'object', NO_Default, IS_Optional  ),
	 CallbackDoc( "callback", "Read callback function", [
		ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
		ParamDoc( "buffer", "The filecontent", "string", NO_Default, IS_Obligated) ])
	],

	ReturnDoc( "The file's content", 'string')
)

