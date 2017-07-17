# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

ClassDoc( "File", "File handling class.",
    [ SeeDoc( "fs" ), SeeDoc( "process.cwd"), SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ) ],
    [ ExampleDoc( """File.read("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
    console.log( buffer );
});""" ) ],
    NO_Inherrits,
    NO_Extends,
    products=["Frontend", "Server"]
)

FieldDoc( "File.filesize", "The size of the file in bytes.",
    [ SeeDoc( "File.filename" ) ],
    [ ExampleDoc( """File.read("tmp_0.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
    console.log(this.filename + " " + this.filesize + " bytes");
});""" ) ],
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
    NO_Default
)

FieldDoc( "File.filename", "The name of the file.",
    [ SeeDoc( "File.filesize" ) ],
    [ ExampleDoc( """var f = new File("foobar");
console.log(f.filename);""" ) ],
    IS_Dynamic, IS_Public, IS_Readonly,
    'string',
    NO_Default
)

ConstructorDoc( "File", "Constructor for a File object that can do operations on files.",
    [ SeeDoc( "global.__filename" ), SeeDoc( "global.__dirname" ), SeeDoc( "process.cwd" ), SeeDoc( "fs" ) ],
    [ ExampleDoc( """var f = new File("tmp_0.txt", { encoding: "utf8" });
f.open("w", function(err) {
    if (!err) {
        f.write("hello", function() {});
        // Operations are queued, the file will only be closed after the write() operation
        f.close();
    }
});""" ) ],
    [
        ParamDoc( "path", "File path relative to the NML (Frontend) or the current working directory (Server)", "string", NO_Default, IS_Obligated ),
        ParamDoc( "options", "Options object (encoding)", ObjectDoc([("encoding", "encoding string (e.g. 'utf8')", "string")]), NO_Default, IS_Optional ),
        CallbackDoc( "callback", "Read callback function", [
            ParamDoc( "err", "Error description or `null`", "string", 'null', IS_Obligated),
            ParamDoc( "buffer", "The file content", "string|ArrayBuffer", NO_Default, IS_Obligated)
        ])
    ],
    ReturnDoc( "FileHandler", "File" )
)

FunctionDoc( "File.write", "Writes a `string` or `arraybuffer` to a file.",
    [SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ) ],
    [ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.open("w+", function(err) {
    if (err) return;

    f.write("hello", function(err) {
        if (err) return;

        f.seek(0, function(err) {
            if (err) return;

            f.read(5, function(err, buffer) {
                console.log(buffer);
                f.close();
            });
        });
    })
});""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "buffer", "The content to write to the file", 'string|ArrayBuffer', NO_Default, IS_Obligated ),
        CallbackDoc( "callback", "Read callback function", [
            ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
        ], NO_Default, IS_Obligated)
    ],
    NO_Returns
)

FunctionDoc("File.writeSync", "Writes a `string` or `arraybuffer` to a file in a synchronous way.",
    SeesDocs( "File.openSync|File.readSync|File.seekSync|File.write" ),
    [ExampleDoc( """var f = new File("foo.txt", { encoding: "utf8" });
f.openSync("w+");
f.writeSync("Hello world!");
console.log(f.readySync());
f.close();""") ],
    IS_Dynamic, IS_Public, IS_Slow,
    [
        ParamDoc( "buffer", "The content to write to the file", 'string|ArrayBuffer', NO_Default, IS_Obligated ),
    ],
    NO_Returns
)

FunctionDoc( "File.isDir", """Determines if the file is a directory or a file.

> The file needs to be opened before calling this method""",
    SeesDocs( "File.rmrf|File.listFiles|File.open|fs|process.cwd" ),
    [ExampleDoc( """var f = new File(".", {encoding: "utf8"});
f.open(function(err) {
    if (!err) {
        console.log(f.filename + "is a " + (f.isDir() ? "directory" : "file"));
    }
});""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    ReturnDoc( "Returns `true` if the filename exists and is a directory, `false` otherwise", "boolean" )
)

FunctionDoc( "File.rm", "Remove a file.",
    SeesDocs( "File.rmrf|fs" ),
    [ExampleDoc( """var f = new File("foobar", {encoding: "utf8"});
f.rm();""" ) ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "File.rmrf", "Deletes the directory and it's content.",
    [SeeDoc( "File.isDir" ), SeeDoc( "File.listFiles" ), SeeDoc( "fs" ), SeeDoc( "process.cwd" ) ],
    [ExampleDoc( """var f = new File("foobar/", {encoding: "utf8"});
if  (f.isDir()) {
    f.rmrf();
}""", run_code=False ) ],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "File.listFiles", """List the files in a directory.

> The type of the entry can only be: f'ile', 'dir', 'other', or 'unknown'.
> The file needs to be opened before calling this method""",
    [SeeDoc( "File.isDir" ), SeeDoc( "File.rmrf" ), SeeDoc( "fs" ), SeeDoc( "process.cwd" ) ],
    [ExampleDoc( """var f = new File(".", {encoding: "utf8"});
f.open(function(err) {
    if (err || !f.isDir()) {
        console.log("Error or not a directory");
        return;
    }
    f.listFiles(function(error, entries) {
        if (err) return;

        console.log(JSON.stringify(entries, null, 4));
    });
});
""" ) ],
    IS_Dynamic, IS_Public, IS_Fast,
    [CallbackDoc( "callback", "Read callback function", [
        ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
        ParamDoc( "entries", "Array with objecs describing the file", 
            ObjectDoc([ ("type", "The type of the entry ('file'|'dir'|'other'|'unkown')", "string"),
                        ("name", "The name of the entry", "string"),
            ]), NO_Default, IS_Obligated ),
    ])],
    NO_Returns
)

FunctionDoc( "File.read", """Reads from a file.""",
    SeesDocs( "File.open|File.seek|File.write|File.close|global.load" ),
    [ExampleDoc( """var f = new File(__filename, {encoding: "utf8"});
f.open("r", function(err) {
    if (err) {
        console.log("Failed to open " + __filename + " : "+ err);
        return;
    }

    f.read(this.filesize, function(err, str) {
        if (err) {
            console.log("Failed to read from " + __filename + " : " + err);
            return;
        }

        console.log(str);
    });
});""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "bytes", "Number of bytes to read", 'integer', 0, IS_Obligated ),
        CallbackDoc( "callback", "Read callback function", [
            ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
            ParamDoc( "buffer", "The file content", "string", NO_Default, IS_Obligated)
        ])
    ],
    NO_Returns
)

FunctionDoc( "File.open", """Opens a file.""",
    [ SeeDoc( "File.openSync" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ), SeeDoc( "File.write" ) ],
    [ ExampleDoc( """var f = new File("tmp_0.txt", {encoding: "utf8"});
f.open("r", function(err) {
    if (err) {
        console.log("Failed to open " + __filename + " : "+ err);
        return;
    } else {
        console.log("File " + this.filename + " opened");
    }
    f.close();
});
""")],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "mode", """The `mode` parameter specifies the type of access you require to the file.
| **Mode** | **Description** |
| --- | --- |
| `r` 	| Open text file for reading. The stream is positioned at the beginning of the file. |
| `r+` | Open for reading and writing. The stream is positioned at the beginning of the file. |
| `w`  | Truncate to zero length or create text file for writing.  The stream is positioned at the beginning of the file. |
| `w+` | Open for reading and writing. The file is created if it does not exist, otherwise it is truncated.  The stream is positioned at the beginning of the file. |
| `a` 	| Open for writing. The file is created if it does not exist. The stream is positioned at the end of the file. Subsequent writes to the file will always end up at the then current end of file, irrespective of any intervening fseek(3) or similar. |
| `a+` | Open for reading and writing.  The file is created if it does not exist. The stream is positioned at the end of the file. Subsequent writes to the file will always end up at the then current end of file, irrespective of any intervening fseek(3) or similar. |""", "string", NO_Default, IS_Obligated ),
        CallbackDoc( "callback", "The callback function", [
            ParamDoc( "Error", "Error description", "string", NO_Default, IS_Obligated )
        ])
    ],
    NO_Returns
)

FunctionDoc( "File.seek", "Moves to a certain offset from the beginning of the file.",
    [SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" ), SeeDoc( "File.close" ) ],
    [ExampleDoc( """var f = new File(__filename, {encoding: "utf8"});
f.open("r", function(err) {
    if (err) return;

    f.seek(50, function() {
        f.read(20, function(err, buffer) {
            console.log(buffer);
        });
    })
});""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "offset", "Move to the `offset` from the beginning of the file. The `offset` is expressed in bytes.", 'integer', 0, IS_Obligated ),
        CallbackDoc( "callback", "Read callback function", [
            ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
        ])
    ],
    NO_Returns
)

FunctionDoc( "File.close", "Closes an open file.",
    [SeeDoc( "File.open" ), SeeDoc( "File.read" ), SeeDoc( "File.seek" )],
    [ExampleDoc( """var f = new File("tmp_0.txt", {encoding: "utf8"});
f.open("r", function(err ) {
    if (!err) {
        f.close();
    }
});""")],
    IS_Dynamic, IS_Public, IS_Fast,
    NO_Params,
    NO_Returns
)

FunctionDoc( "File.openSync", "Open a file in a synchronous way.",
    [ SeeDoc( "File.readSync" ), SeeDoc( "File.closeSync" ), SeeDoc( "File.writeSync" ) ],
    [ ExampleDoc( """var f = new File(__filename, {encoding:"utf8"});
f.openSync("r");
var data = f.readSync(1024);
console.log(data);
f.closeSync();""")],
    IS_Dynamic, IS_Public, IS_Slow,
    NO_Params,
    NO_Returns
)

FunctionDoc("File.readSync", "Reads entire file in a synchronous way.",
    [SeeDoc("File.read")],
    [
        ExampleDoc("""var buffer = File.readSync("tmp_0.txt");"""),
        ExampleDoc("""var string = File.readSync("tmp_0.txt", {encoding: "utf8"});\nconsole.log(string);""")
    ],
    IS_Static, IS_Public, IS_Slow,
    [
        ParamDoc("path", "File path relative to the NML (Frontend) or the current working directory (Server)", 'string', NO_Default, IS_Obligated),
        ParamDoc("options", "Options object ", ObjectDoc([
            ("encoding", "encoding string (e.g. 'utf8')", "string")
        ]), NO_Default, IS_Optional)
    ],
    ReturnDoc("The file's content", 'string' )
)

FunctionDoc("File.readSync", """Read a file in a synchronous way.

If the file has not been opened it will be automatically opened.""",
    [SeeDoc("File.openSync"), SeeDoc("File.writeSync"), SeeDoc("File.closeSync")],
    [ExampleDoc( """var f = new File(__filename, {encoding: "utf8"});
var string = f.readSync();
console.log(string);""")],
    IS_Dynamic, IS_Public, IS_Slow,
    [ParamDoc("readSize", "Number of bytes to read", 'integer', 'The size of the file', IS_Optional)],
    ReturnDoc("The file's content", 'string|ArrayBuffer')
)

FunctionDoc("File.seekSync", "Moves to a certain offset from the beginning of the file.",
    SeesDocs( "File.openSync|File.closeSync|File.readSync|File.writeSync|File.seek"),
    [ExampleDoc( """var f = new File(__filename, {encoding: "utf8"});
f.seekSync(10);
console.log(f.readSync(100));""") ],
    IS_Dynamic, IS_Public, IS_Fast,
    [
        ParamDoc( "offset", "Move to the `offset` from the beginning of the file. The `offset` is expressed in bytes.", 'integer', 0, IS_Obligated )
    ],
    NO_Returns
)

FunctionDoc("File.closeSync", "Close a file in a synchronous way.",
    [SeeDoc( "File.openSync" ), SeeDoc( "File.readSync" ), SeeDoc( "File.seekSync" ), SeeDoc("File.writeSync"), SeeDoc("File.close")],
    [ ExampleDoc( """var f = new File("tmp_0.txt", {encoding: "utf8"});
f.openSync();
console.log(f.readSync());
f.closeSync();""")],
    IS_Dynamic, IS_Public, IS_Slow,
    NO_Params,
    NO_Returns
)

FunctionDoc( "File.read", """Reads entire file.

This method also accept `http` file or any protocol supported by Nidium""",
    [SeeDoc( "File.readSync") ],
    [ExampleDoc( """File.read(__filename, function(err, buffer) {
    // buffer is an array buffer containing the data
});""" ),
    ExampleDoc( """File.read(__filename, {encoding: "utf8"}, function(err, data) {
        // data is an UTF8 string containing the data
        console.log(data);
});""" )
    ],
    IS_Static, IS_Public, IS_Fast,
    [
        ParamDoc( "path", "File path relative to the NML (Frontend) or the current working directory (Server)", 'string', NO_Default, IS_Obligated ),
        ParamDoc( "options", "Options object (encoding)", ObjectDoc([("encoding", "encoding string (e.g. 'utf8')", "string")]), NO_Default, IS_Optional ),
        CallbackDoc( "callback", "Read callback function", [
            ParamDoc( "err", "Error description", "string", NO_Default, IS_Obligated ),
            ParamDoc( "buffer", "The file content", "string", NO_Default, IS_Obligated)
        ])
    ],
    ReturnDoc( "The file's content", 'string')
)

