
Tests.register("Console.methods.Console.log.0", function( ) {
	var dummy = 0;
		console.log( 'Nidium' );

	Assert.equal(dummy, 0);
});

Tests.register("Console.methods.Console.info.0", function( ) {
	var dummy = 1;
		console.info( 'Nidium, A new breed of browser' );

	Assert.equal(dummy, 1);
});

Tests.register("Console.methods.Console.error.0", function( ) {
	var dummy = 2;
		console.error( 'Nidium says "no"' );

	Assert.equal(dummy, 2);
});

Tests.register("Console.methods.Console.write.0", function( ) {
	var dummy = 3;
		console.write( 'Nidium', 'A new breed of browser' );

	Assert.equal(dummy, 3);
});

Tests.register("Console.methods.Console.hide.0", function( ) {
	var dummy = 4;
		console.hide( );

	Assert.equal(dummy, 4);
});

Tests.register("Console.methods.Console.warn.0", function( ) {
	var dummy = 5;
		console.warn( 'Nidium, Improving the web' );

	Assert.equal(dummy, 5);
});

Tests.register("Console.methods.Console.show.0", function( ) {
	var dummy = 6;
		console.show( );

	Assert.equal(dummy, 6);
});

Tests.register("Console.methods.Console.clear.0", function( ) {
	var dummy = 7;
		console.clear( );

	Assert.equal(dummy, 7);
});

Tests.register("Console.base.Console.0", function( ) {
	var dummy = 8;
		console.log( 'Nidium' );

	Assert.equal(dummy, 8);
});

Tests.register("fs.methods.fs.readDir.0", function( ) {
	var dummy = 9;
		fs.readDir( function( err, entries ) {
				console.log( JSON.stringify( entries ) ) ; } );

	Assert.equal(dummy, 9);
});

Tests.register("fs.base.fs.0", function( ) {
	var dummy = 10;
		fs.readDir( function( err, entries ) {
				console.log( JSON.stringify( entries ) ); } );

	Assert.equal(dummy, 10);
});

Tests.register("Http.constructors.Http.0", function( ) {
	var dummy = 11;
		var url = "http://www.somewhere.com/image.png";
		var params = { foo : "bar", id : 14 };
		var r = new Http(url);
		r.ondata = function(e){
		    // get the % loaded with e.total and e.read
		    var p = Number(e.total) != 0 ? Number(e.read) * 100 / e.total : 0;
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

	Assert.equal(dummy, 11);
});

Tests.register("Http.base.Http.0", function( ) {
	var dummy = 12;
		var url = "http://www.somewhere.com/image.png";
		var params = { foo : "bar", id : 14 };
		var h = new Http(url, params, function(e){
			for (var h in e.headers){
				console.log(h, e.headers[h]);
			}
			console.log("Complete");
		});
		h.ondata = function(e){
			console.log(e.percent +"%");
		};
		h.onerror = function(e){
			console.log(e.error);
		};

	Assert.equal(dummy, 12);
});

Tests.register("HTTPListener.base.HTTPListener.0", function( ) {
	var dummy = 13;
		var ws = new HTTPListener(  8080, true, "127.0.0.1" );
		ws.onrequest = function( request, client ) {
			console.log( request.method + " " + request.url );
			console.log( JSON.stringify( request.headers ) );
			console.log( JSON.stringify( client ) );
			client.write( "hello, this is nidium" );
		} ;

	Assert.equal(dummy, 13);
});

Tests.register("global.properties.global.Debug.0", function( ) {
	var dummy = 14;
		var d = {a:1, b: "a"};
		var s = Debug.serialize(d);
		var u = Debug.unserialize(s)
		console.log(JSON.stringify(u));

	Assert.equal(dummy, 14);
});

Tests.register("global.methods.global.setImmediate.0", function( ) {
	var dummy = 15;
		var t = setImmediate( function(){
			console.log( "Nidium" );
		});

	Assert.equal(dummy, 15);
});

Tests.register("global.methods.global.clearInterval.0", function( ) {
	var dummy = 16;
		var t = setInterval( function() {
			console.log( "Nidium" );
		}, 1000 );
		clearInterval( t );

	Assert.equal(dummy, 16);
});

Tests.register("global.methods.global.pwd.0", function( ) {
	var dummy = 17;
		console.log( pwd( ) );

	Assert.equal(dummy, 17);
});

Tests.register("global.methods.global.setTimeout.0", function( ) {
	var dummy = 18;
		var t = setTimeout ( console.log, 1000, "Nidium" );
		clearTimeout( t );

	Assert.equal(dummy, 18);
});

Tests.register("global.methods.global.setInterval.0", function( ) {
	var dummy = 19;
		var t = setInterval( function() {
			console.log( "Nidium" );
		}, 1000 );
		clearInterval( t );

	Assert.equal(dummy, 19);
});

Tests.register("global.methods.global.load.0", function( ) {
	var dummy = 20;
		try {
			load( '/nidium.js' ); 
		} catch(e) {
			console.log("warning: "+ e.message);
			
			}

	Assert.equal(dummy, 20);
});

Tests.register("global.methods.global.clearTimeout.0", function( ) {
	var dummy = 21;
		var t = setTimeout( function() {
			console.log( "Nidium" );
		}, 1000 );
		clearTimeout( t );

	Assert.equal(dummy, 21);
});

Tests.register("global.methods.global.btoa.0", function( ) {
	var dummy = 22;
		console.log(btoa("Hello Nidium"));

	Assert.equal(dummy, 22);
});

Tests.register("global.base.global.0", function( ) {
	var dummy = 23;
		load( 'script.js' );

	Assert.equal(dummy, 23);
});

Tests.register("global.base.global.1", function( ) {
	var dummy = 24;
		console.log( pwd( )  + '\n' + __dirname + '\n' + __filename + '\n' );

	Assert.equal(dummy, 24);
});

Tests.register("global.base.global.2", function( ) {
	var dummy = 25;
		var t = setTimeout( function() {
			console.log( "Nidium" );}, 1000 );
		clearTimeout( t );

	Assert.equal(dummy, 25);
});

Tests.register("global.base.global.3", function( ) {
	var dummy = 26;
		var t = setInterval( console.log, 1000, "Nidium" );
		clearInterval( t );

	Assert.equal(dummy, 26);
});

Tests.register("Socket.properties.Socket.binary.0", function( ) {
	var dummy = 27;
		var socket = new Socket("0.0.0.0", 8001).listen()
		socket.binary = true;
		socket.onread = function(clientSocket, data) {
		    console.log("=>", data); // data is an ArrayBuffer
		}

	Assert.equal(dummy, 27);
});

Tests.register("Socket.properties.Socket.readline.0", function( ) {
	var dummy = 28;
		var socket = new Socket("irc.freenode.org", 6667).connect();
		socket.readline = true;
		socket.onconnect = function(clientSocket) {
		    clientSocket.write("NICKNAME foo\n");
		    clientSocket.write("USER A A A A\n");
		}
		// if the server sends something like :
		// HELLO\nPING 123\nFOOBAR, the "onread" callback is called twice (once with HELLO and once with PING).
		// The remaining data (FOOBAR) is held in memory until a "\n" is encountered.
		socket.onread = function(data) {
		    console.log("=>", data);
		}

	Assert.equal(dummy, 28);
});

Tests.register("Socket.properties.Socket.encoding.0", function( ) {
	var dummy = 29;
		var socket = new Socket("127.0.0.1", 6667).connect();
		socket.binary = false;
		socket.encoding = 'utf8';
		// if the server sends something like :
		socket.onread = function(data) {
		    console.log("=>", data);
		}

	Assert.equal(dummy, 29);
});

Tests.register("Socket.constructors.Socket.0", function( ) {
	var dummy = 30;
		//Client
		var client = new Socket("google.com", 80);
		client.connect();

	Assert.equal(dummy, 30);
});

Tests.register("Socket.constructors.Socket.1", function( ) {
	var dummy = 31;
		//Server
		var server = new Socket("0.0.0.0", 8005);
		server.listen();

	Assert.equal(dummy, 31);
});

Tests.register("Socket.methods.Socket.listen.0", function( ) {
	var dummy = 32;
		var socket = new Socket("0.0.0.0", 8006).listen();
		
		socket.onaccept = function(clientSocket) {
		    clientSocket.write("hello !\n");
		    clientSocket.disconnect();
		}

	Assert.equal(dummy, 32);
});

Tests.register("Socket.methods.Socket.sendTo.0", function( ) {
	var dummy = 33;
		var s = new Socket("0.0.0.0", 8008).listen("udp");
		
		s.onaccept = function(clientSocket) {
		    clientSocket.sendTo( clientSocket.ip, clientSocket.port, "hello !\n");
		    clientSocket.disconnect();
		}

	Assert.equal(dummy, 33);
});

Tests.register("Socket.methods.Socket.connect.0", function( ) {
	var dummy = 34;
		var client = new Socket("nidium.com", 80).connect();

	Assert.equal(dummy, 34);
});

Tests.register("Socket.methods.Socket.connect.1", function( ) {
	var dummy = 35;
		var client = new Socket("google.com", 80).connect("ssl");

	Assert.equal(dummy, 35);
});

Tests.register("Socket.base.Socket.0", function( ) {
	var dummy = 36;
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
		    //console.log("=>", data);
		}

	Assert.equal(dummy, 36);
});

Tests.register("Socket.base.Socket.1", function( ) {
	var dummy = 37;
		//Server example
		var socket = new Socket("0.0.0.0", 8000).listen();
		socket.onaccept = function(clientSocket) {
		    clientSocket.write("Hello");
		}
		socket.ondisconnect = function(clientSocket) {
		    console.log("Client Disconnected");
		}
		socket.onread = function(clientSocket, data) {
		    //console.log("=>", data);
		}

	Assert.equal(dummy, 37);
});

Tests.register("Socket.events.Socket.onconnect.0", function( ) {
	var dummy = 38;
		var socket = new Socket("0.0.0.0", 8002).listen();
		socket.onaccept = function(clientSocket) {
		    clientSocket.write("hello !\n");
		}

	Assert.equal(dummy, 38);
});

Tests.register("Socket.events.Socket.onconnect.1", function( ) {
	var dummy = 39;
		var socket = new Socket("irc.freenode.org", 6667).connect();
		socket.onconnect = function( clientSocket ) {
		    clientSocket.write("NICKNAME foo\n");
		    clientSocket.write("USER A A A A\n");
		}

	Assert.equal(dummy, 39);
});

Tests.register("Socket.events.Socket.onaccept.0", function( ) {
	var dummy = 40;
		var socket = new Socket("0.0.0.0", 8003).listen();
		socket.onaccept = function(clientSocket) {
		    clientSocket.write("hello !\n");
		}

	Assert.equal(dummy, 40);
});

Tests.register("Socket.events.Socket.onmessage.0", function( ) {
	var dummy = 41;
		var socket = new Socket("0.0.0.0", 8004).listen("udp");
		socket.onmessage = function(data, details) {
		    console.log("A message has arrived from", details.ip, ":", details.port);
		    console.log("=>", data);
		}

	Assert.equal(dummy, 41);
});

Tests.register("Thread.methods.Thread.start.0", function( ) {
	var dummy = 42;
		document.status.open();
		
		var t = new Thread(function(...n){
		    var p = 0;
		    for (var i = 0; i < 20000000; i++) {
		        if (i % 10000 == 0) this.send(i);
		        p++;
		    }
		    return n;
		});
		t.onmessage = function(e){
		    var i = e.data,
		        v = i * 100 / 20000000;
		
		    document.status.label = Math.round(v) + "%";
		    document.status.value = v;
		};
		t.oncomplete = function(e){
		    if (e.data){
		        console.log("i'm done with", e.data);
		    }
		    document.status.close();
		};
		t.start(5, 6, 6, 9);

	Assert.equal(dummy, 42);
});

Tests.register("Thread.base.Thread.0", function( ) {
	var dummy = 43;
		document.status.open();
			var t = new Thread(function(foo){
		   // something loud and heavy
		});
		t.oncomplete = function(e){
		   // executed when the job is done
		   console.log(e.data);
		};
		t.start("bar"); // start the new job with "bar" as a parameter.

	Assert.equal(dummy, 43);
});

Tests.register("SocketClient.methods.SocketClient.disconnect.0", function( ) {
	var dummy = 44;
		var s = new Socket("0.0.0.0", 8007).listen();
		
		s.onaccept = function(clientSocket) {
		    clientSocket.write("hello !\n");
		    clientSocket.disconnect();
		}

	Assert.equal(dummy, 44);
});

Tests.register("SocketClient.methods.SocketClient.disconnect.1", function( ) {
	var dummy = 45;
		var socket = new Socket("irc.freenode.org", 6667).connect();
		
		socket.onconnect = function(clientSocket) {
		    clientSocket.write("NICKNAME foo\n");
		    clientSocket.write("USER A A A A\n");
		    clientSocket.disconnect()
		}

	Assert.equal(dummy, 45);
});

Tests.register("SocketClient.events.SocketClient.ondisconnect.0", function( ) {
	var dummy = 46;
		var socket = new Socket("irc.freenode.org", 6667).connect();
		socket.onconnect = function(clientSocket) {
		    clientSocket.write("NICKNAME foo\n");
		    clientSocket.disconnect( );
		}
		socket.ondisconnect=  function( clientSocket ) {
			console.log( "Disconnected from: " + clientSocket.ip );
		}

	Assert.equal(dummy, 46);
});

Tests.register("NativeDebug.methods.NativeDebug.serialize.0", function( ) {
	var dummy = 47;
		var d = {a:1, b: "a"};
		var s = Debug.serialize(d);
		var u = Debug.unserialize(s)
		console.log(JSON.stringify(u));

	Assert.equal(dummy, 47);
});

Tests.register("NativeDebug.methods.NativeDebug.unserialize.0", function( ) {
	var dummy = 48;
		var d = {a:1, b: "a"};
		var s = Debug.serialize(d);
		var u = Debug.unserialize(s)
		console.log(JSON.stringify(u));

	Assert.equal(dummy, 48);
});

Tests.register("NativeDebug.base.NativeDebug.0", function( ) {
	var dummy = 49;
		var d = {a:1, b: "a"};
		var s = Debug.serialize(d);
		var u = Debug.unserialize(s)
		console.log(JSON.stringify(u));

	Assert.equal(dummy, 49);
});

Tests.register("Stream.constructors.Stream.0", function( ) {
	var dummy = 50;
		var s = new Stream( "http://www.nidium.com" );
		s.onavailabledata = function( ) {
			s.seek( 80 );
			s.stop( );
		}
		s.start( 1024 );

	Assert.equal(dummy, 50);
});

