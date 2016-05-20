# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "fs", "Filesystem access.",
	[ SeeDoc( "process.cwd" ), SeeDoc( "File"  ) ],
	[ ExampleDoc( """fs.readDir( function( err, entries ) {
		console.log( JSON.stringify( entries ) ); } );""") ]
)

FunctionDoc( "fs.readDir", "Read the content of a directory.",
	[ SeeDoc( "process.cwd" ), SeeDoc( "File.isDir" ),
	 SeeDoc( "global.listDir" ), SeeDoc( "File.rmrf" ) ],
	[ ExampleDoc( """fs.readDir( function( err, entries ) {
		console.log( JSON.stringify( entries ) ) ; } );""") ] ,
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

