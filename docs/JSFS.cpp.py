# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "fs", "Filesystem access.",
    [ SeeDoc( "process.cwd" ), SeeDoc( "File"  ) ],
    [ ExampleDoc( """fs.readDir( ".", function( err, entries ) {
        console.log( JSON.stringify( entries ) ); } );""") ],
    products=["Frontend", "Server"]
)

FunctionDoc( "fs.readDir", "Read the content of a directory.",
    [ SeeDoc( "process.cwd" ), SeeDoc( "File.isDir" ),
     SeeDoc( "fs.readDir" ), SeeDoc( "File.rmrf" ) ],
    [ ExampleDoc( """fs.readDir( ".", function( err, entries ) {
        console.log( JSON.stringify( entries ) ) ; } );""") ] ,
    IS_Static, IS_Public, IS_Fast,
    [
        ParamDoc("dir", "Directory relative to the 'start' point (for frontend: the nml file, for server this is the js file on the commandline", 'string', NO_Default, IS_Obligated),
        CallbackDoc( 'fn', 'The function to be called', [
            ParamDoc( 'err', 'error', 'integer', NO_Default, IS_Optional ),
            ParamDoc( 'entry', 'Object describing the file listing', ObjectDoc([("name", "The filename", "string")]),NO_Default, IS_Obligated)
        ])
    ],
    NO_Returns
)

