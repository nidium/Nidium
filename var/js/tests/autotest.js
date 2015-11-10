var COUNTERS = { examples: 0, fails : 0};
try {

test_Console_methods_Console_log_0 = function() {
console.log( 'Nidium' );
};

test_Console_methods_Console_info_0 = function() {
console.info( 'Nidium, A new breed of browser' );
};

test_Console_methods_Console_error_0 = function() {
console.error( 'Nidium, Cannot display HTML' );
};

test_Console_methods_Console_warn_0 = function() {
console.warn( 'Nidium, Improving the web' );
};

test_Console_base_Console_0 = function() {
console.log( 'Nidium' );
};

test_global_methods_global_clearInterval_0 = function() {
var t = setInterval ( 'console.log( "Nidium" );', 1000 );
clearInterval( t );
};

test_global_methods_global_pwd_0 = function() {
console.log( pwd( ) );
};

test_global_methods_global_setTimeout_0 = function() {
var t = setTimeout ( 'console.log( "Nidium" );', 1000 );
clearTimeout( t );
};

test_global_methods_global_setInterval_0 = function() {
var t = setInterval( 'console.log( "Nidium" );', 1000 );
clearInterval( t );
};

test_global_methods_global_load_0 = function() {
load( '/nidium.js' );
};

test_global_methods_global_clearTimeout_0 = function() {
var t = setTimeout ( 'console.log( "Nidium" );', 1000 );
clearTimeout( t );
};

test_global_base_global_0 = function() {
load( 'script.js' );
};

test_global_base_global_1 = function() {
console.log( pwd( )  + ' ' + __dirname + ' ' + __filename );
};

test_global_base_global_2 = function() {
var t = setTimeout( 'console.log( "Nidium" );', 1000 );
clearTimeout( t );
};

test_global_base_global_3 = function() {
var t = setInterval( 'console.log( "Nidium" );', 1000 );
clearInterval( t );
};

test_Http_constructors_Http_0 = function() {
var url = "http://www.somewhere.com/image.png";
var params = { foo : "bar", id : 14 };
var r = new Http(url);
r.ondata = function(e){
    // get the % loaded with e.total and e.read
    var p = Number(e.total) !=0 ? Number(e.read)*100/e.total : 0;
    var size = (e.type == "binary" ? e.data.byteLength : e.data.length);
    var percent = Math.round(p*100)/100;
};
r.onerror = function(e){
    console.log('Error: ' + e.error);
};
var options = {
    method : "POST",
    headers : {
        "User-Agent" : "some user agent"
    },
    data : {
        foo : 4,
        bar : 8
    }
};
r.request(options, function(e){
    console.log("Complete.");
    for (var h in e.headers){
        console.log(h + " : " + e.headers[h]);
    }
});
};

test_Http_base_Http_0 = function() {
var url = "http://www.somewhere.com/image.png";
var params = { foo : "bar", id : 14 };
var h = new Http.get(url, params, function(e){
	for (var h in e.headers){
		console.log(h, e.headers[h]);
	}
	console.log("Complete");
});
h.ondata = function(e){
	console.log(e.percent+"%");
};
h.onerror = function(e){
	console.log(e.error);
};
};

test_HTTPListener_base_HTTPListener_0 = function() {
var ws = new HTTPListener(  8080, true, "127.0.0.1" );
ws.onrequest = function( request, client ) {
	console.log( request.method + " " + request.url );
	console.log( JSON.stringify( request.headers ) );
	console.log( JSON.stringify( client ) );
	client.write( "hello, this is nidium" );
} ;
};

test_fs_methods_fs_readDir_0 = function() {
fs.readDir( function( err, entries ) { 
		console.log( JSON.stringify( entries ) ) ; } );
};

test_fs_base_fs_0 = function() {
fs.readDir( function( err, entries ) { 
		console.log( JSON.stringify( entries ) ); } );
};

test_Socket_properties_Socket_binary_0 = function() {
var socket = new Socket("0.0.0.0", 8000).listen()
socket.binary = true;
socket.onread = function(client, data) {
    console.log("=>", data); // data is an ArrayBuffer
}
};

test_Socket_properties_Socket_readline_0 = function() {
var socket = new Socket("irc.freenode.org", 6667).connect();
socket.readline = true;
socket.onconnect = function() {
    client.write("NICKNAME foo\n");
    client.write("USER A A A A\n");
}
// if the server sends something like :
// HELLO\nPING 123\nFOOBAR, the "onread" callback is called twice (once with HELLO and once with PING).
// The remaining data (FOOBAR) is held in memory until a "\n" is encountered.
socket.onread = function(data) {
    console.log("=>", data);
}
};

test_Socket_properties_Socket_encoding_0 = function() {
var socket = new Socket("127.0.0.1", 6667).connect();
socket.binary = false;
socket.encoding = 'utf8';
// if the server sends something like :
socket.onread = function(data) {
    console.log("=>", data);
}
};

test_Socket_constructors_Socket_0 = function() {
//Client
var client = new Socket("google.com", 80);
client.connect();
};

test_Socket_constructors_Socket_1 = function() {
//Server
var server = new Socket("0.0.0.0", 8000);
server.listen();
};

test_Socket_methods_Socket_listen_0 = function() {
var socket = new Socket("0.0.0.0", 8000).listen();

socket.onaccept = function(client) {
    client.write("hello !\n");
    client.disconnect();
}
};

test_Socket_methods_Socket_sendTo_0 = function() {
var s = new Socket("0.0.0.0", 8000).listen("udp");

s.onaccept = function(client) {
    client.sendTo( client.ip, client.port, "hello !\n");
    client.disconnect();
}
};

test_Socket_methods_Socket_connect_0 = function() {
var client = new Socket("nidium.com", 80).connect();
};

test_Socket_methods_Socket_connect_1 = function() {
var client = new Socket("google.com", 80).connect("ssl");
};

test_Socket_base_Socket_0 = function() {
//Client Example
var socket = new Socket("nidium.com", 80).connect();
socket.onconnect = function() {
    this.write("GET / HTTP/1.1\n");
    this.write("Host: www.nidium.com\n\n");
}
socket.ondisconnect = function() {
    console.log("Disconnected");
}
socket.onread = function(data) {
    console.log("=>", data);
}
};

test_Socket_base_Socket_1 = function() {
//Server example
var socket = new Socket("0.0.0.0", 8000).listen();
socket.onaccept = function(client) {
    client.write("Hello");
}
socket.ondisconnect = function(client) {
    console.log("Client Disconnected");
}
socket.onread = function(client, data) {
    console.log("=>", data);
}
};

test_Socket_events_Socket_onconnect_0 = function() {
var socket = new Socket("0.0.0.0", 8000).listen();
socket.onaccept = function(client) {
    client.write("hello !\n");
}
};

test_Socket_events_Socket_onconnect_1 = function() {
var socket = new Socket("irc.freenode.org", 6667).connect();
socket.onconnect = function( clientSocket ) {
    client.write("NICKNAME foo\n");
    client.write("USER A A A A\n");
}
};

test_Socket_events_Socket_onaccept_0 = function() {
var socket = new Socket("0.0.0.0", 8000).listen();
socket.onaccept = function(client) {
    client.write("hello !\n");
}
};

test_Socket_events_Socket_onmessage_0 = function() {
var socket = new Socket("0.0.0.0", 8000).listen("udp");
socket.onmessage = function(data, details) {
    console.log("A message has arrived from", details.ip, ":", details.port);
    console.log("=>", data);
}
};

test_Thread_methods_Thread_start_0 = function() {
document.status.open();

var t = new Thread(function(...n){
    var p = 0;
    for (var i=0; i<20000000; i++){
        if (i%10000 == 0) this.send(i);
        p++;
    }
    return n;
});
t.onmessage = function(e){
    var i = e.data,
        v = i*100/20000000;

    document.status.label = Math.round(v)+"%";
    document.status.value = v;
};
t.oncomplete = function(e){
    if (e.data){
        console.log("i'm done with", e.data);
    }
    document.status.close();
};
t.start(5, 6, 6, 9);
};

test_Thread_base_Thread_0 = function() {
document.status.open();
	var t = new Thread(function(foo){
   // something loud and heavy
});
t.oncomplete = function(e){
   // executed when the job is done
   console.log(e.data);
};
t.start("bar"); // start the new job with "bar" as a parameter.
};

test_SocketClient_methods_SocketClient_disconnect_0 = function() {
var s = new Socket("0.0.0.0", 8000).listen();

s.onaccept = function(client) {
    client.write("hello !\n");
    client.disconnect();
}
};

test_SocketClient_methods_SocketClient_disconnect_1 = function() {
var socket = new Socket("irc.freenode.org", 6667).connect();

socket.onconnect = function() {
    client.write("NICKNAME foo\n");
    client.write("USER A A A A\n");
    client.disconnect()
}
};

test_SocketClient_events_SocketClient_ondisconnect_0 = function() {
var socket = new Socket("irc.freenode.org", 6667).connect();
socket.onconnect = function() {
    client.write("NICKNAME foo\n");
    client.disconnect( );
}
socket.ondisconnect=  function( client ) {
	console.log( "Disconnected from: " + client.ip );
}
};

test_NativeDebug_methods_NativeDebug_serialize_0 = function() {
var d = NativeDebug( { wtf: 'nidium' }  );
};

test_NativeDebug_methods_NativeDebug_unserialize_0 = function() {
var d = NativeDebug( { wtf: 'nidium' }  );
};

test_Stream_constructors_Stream_0 = function() {
var s = new Stream( "http://www.nidium.com" );
s.onavailabledata = function( ) {
	s.seek( 80 );
	s.stop( );
}
s.start( 1024 );
};

test_NativeProcess_constructors_NativeProcess_0 = function() {
var p = NativeProcess( "wget", "-m www.nidium.com" );
};

test_NativeProcess_base_NativeProcess_0 = function() {
var p = NativeProcess( "wget", "-m www.nidium.com" );
};

test_File_properties_File_filesize_0 = function() {
File.readFile("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
	console.log( this.filename + " "+ this.filesize + " bytes" );
});
};

test_File_properties_File_filename_0 = function() {
File.readFile("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
	console.log( this.filename + " "+ this.filesize + " bytes" );
});
};

test_File_constructors_File_0 = function() {
var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function(err) {
    if (!err) {
        f.write("hello", function() {});
        f.close();
    }
});
};

test_File_methods_File_listFiles_0 = function() {
var f = new File( "/tmp/", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.listFiles( function( error, entrys ) {
				if ( ! err ) {
					console.log( JSON.stringify( entries ) );
			}
		} );
		}
};

test_File_methods_File_isDir_0 = function() {
var f = new File( "/tmp/imp.red", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.rmrf( )
		}
};

test_File_methods_File_readSync_0 = function() {
var buffer = File.readSync("foo.txt");
};

test_File_methods_File_readSync_1 = function() {
var string = File.readFileSync("foo.txt", { encoding: "utf8" });
};

test_File_methods_File_seek_0 = function() {
var f = new File("foo.txt", { encoding: "utf8" });
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
});
};

test_File_methods_File_closeSync_0 = function() {
var f = new File( "foo.txt", { encoding: "utf8" } );
f.openSync( "rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function () { } );
        f.closeSync( );
    }
});
};

test_File_methods_File_read_0 = function() {
var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function(err) {
    if (!err) {
        f.write("hello", function() {});
        f.close();
    }
});
};

test_File_methods_File_read_1 = function() {
File.read("foo.txt", function(err, buffer) {
    // buffer is an array buffer containing the data
});
};

test_File_methods_File_read_2 = function() {
File.read("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
});
};

test_File_methods_File_read_3 = function() {
File.read("http://www.example.com/foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
});
};

test_File_methods_File_close_0 = function() {
var f = new File("foo.txt", { encoding: "utf8" });
f.open("rw", function( err ) {
	if ( ! err ) {
		f.close( );
	}
});
};

test_File_methods_File_close_1 = function() {
var f = new File.openSync( "foo.txt", { encoding: 'utf8' } );
f.close( );
};

test_File_methods_File_write_0 = function() {
var f = new File("foo.txt", { encoding: "utf8" });
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
});
};

test_File_methods_File_readFile_0 = function() {
File.readFile("foo.txt", function(err, buffer) {
    // buffer is an array buffer containing the data
	});
};

test_File_methods_File_readFile_1 = function() {
File.readFile("foo.txt", {encoding: "utf8"}, function(err, buffer) {
		// buffer is an UTF8 string containing the data
	});
};

test_File_methods_File_readFile_2 = function() {
//readFile can be used with any protocol supported by nidium (e.g. http):
	File.readFile("http://www.example.com/foo.txt", {encoding: "utf8"}, function(err, buffer) {
		// buffer is an UTF8 string containing the data
	});
};

test_File_methods_File_open_0 = function() {
var f = new File( "foo.txt", { encoding: "utf8" } );
f.open( "rw", function( err ) {
    if ( ! err ) {
        f.write( "hello", function () { } );
        f.close( );
    }
});
};

test_File_methods_File_rmrf_0 = function() {
var f = new File( "/tmp/dangerous/", { encoding: "utf8" } );
		if  ( f.isDir( ) ) {
			f.rmrf( )
		}
};

test_File_base_File_0 = function() {
File.readFile("foo.txt", {encoding: "utf8"}, function(err, buffer) {
    // buffer is an UTF8 string containing the data
    console.log( buffer );
});
};

} catch( err ) {
	COUNTERS.fails = 1;
	console.log('Syntax error in example code; Go fix that!' + err.message );
}
if ( ! COUNTERS.fails ) {
	try {
		var fns = ['test_Console_methods_Console_log_0', 'test_Console_methods_Console_info_0', 'test_Console_methods_Console_error_0', 'test_Console_methods_Console_warn_0', 'test_Console_base_Console_0', 'test_global_methods_global_clearInterval_0', 'test_global_methods_global_pwd_0', 'test_global_methods_global_setTimeout_0', 'test_global_methods_global_setInterval_0', 'test_global_methods_global_load_0', 'test_global_methods_global_clearTimeout_0', 'test_global_base_global_0', 'test_global_base_global_1', 'test_global_base_global_2', 'test_global_base_global_3', 'test_Http_constructors_Http_0', 'test_Http_base_Http_0', 'test_HTTPListener_base_HTTPListener_0', 'test_fs_methods_fs_readDir_0', 'test_fs_base_fs_0', 'test_Socket_properties_Socket_binary_0', 'test_Socket_properties_Socket_readline_0', 'test_Socket_properties_Socket_encoding_0', 'test_Socket_constructors_Socket_0', 'test_Socket_constructors_Socket_1', 'test_Socket_methods_Socket_listen_0', 'test_Socket_methods_Socket_sendTo_0', 'test_Socket_methods_Socket_connect_0', 'test_Socket_methods_Socket_connect_1', 'test_Socket_base_Socket_0', 'test_Socket_base_Socket_1', 'test_Socket_events_Socket_onconnect_0', 'test_Socket_events_Socket_onconnect_1', 'test_Socket_events_Socket_onaccept_0', 'test_Socket_events_Socket_onmessage_0', 'test_Thread_methods_Thread_start_0', 'test_Thread_base_Thread_0', 'test_SocketClient_methods_SocketClient_disconnect_0', 'test_SocketClient_methods_SocketClient_disconnect_1', 'test_SocketClient_events_SocketClient_ondisconnect_0', 'test_NativeDebug_methods_NativeDebug_serialize_0', 'test_NativeDebug_methods_NativeDebug_unserialize_0', 'test_Stream_constructors_Stream_0', 'test_NativeProcess_constructors_NativeProcess_0', 'test_NativeProcess_base_NativeProcess_0', 'test_File_properties_File_filesize_0', 'test_File_properties_File_filename_0', 'test_File_constructors_File_0', 'test_File_methods_File_listFiles_0', 'test_File_methods_File_isDir_0', 'test_File_methods_File_readSync_0', 'test_File_methods_File_readSync_1', 'test_File_methods_File_seek_0', 'test_File_methods_File_closeSync_0', 'test_File_methods_File_read_0', 'test_File_methods_File_read_1', 'test_File_methods_File_read_2', 'test_File_methods_File_read_3', 'test_File_methods_File_close_0', 'test_File_methods_File_close_1', 'test_File_methods_File_write_0', 'test_File_methods_File_readFile_0', 'test_File_methods_File_readFile_1', 'test_File_methods_File_readFile_2', 'test_File_methods_File_open_0', 'test_File_methods_File_rmrf_0', 'test_File_base_File_0'];
		for (var i in fns ) {
			console.log('running: ' + fns[i] );
			global[fns[i]]();
			COUNTERS.examples++;
		}
	} catch ( err ) {
		console.log( err.message );
		COUNTERS.fails++;
	}
	if ( COUNTERS.fails > 0 ) {
		console.log( COUNTERS.fails + ' examples did not run correctly! Go fix them!' );
	} else {
		console.log( "These " + COUNTERS.examples + " examples seem to be ok!" );
	}
}
