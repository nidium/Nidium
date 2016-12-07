
Tests.register("Console.methods.Console.log.0", function( ) {
	var dummy = 0;
		console.log( 'Nidium' );

	Assert.equal(dummy, 0);
});

Tests.register("Console.methods.Console.profile.0", function( ) {
	var dummy = 1;
		console.profile( );
			// Time intense task
			var result = Console.profileEnd( );
			console.log( JSON.stringify( result ) );

	Assert.equal(dummy, 1);
});

Tests.register("Console.methods.Console.error.0", function( ) {
	var dummy = 2;
		console.error( 'Nidium, Cannot display HTML' );

	Assert.equal(dummy, 2);
});

Tests.register("Console.methods.Console.profileEnd.0", function( ) {
	var dummy = 3;
		console.profile( );
			// Time intense task
			var result = Console.profileEnd( );
			console.log( JSON.stringify( result ) );

	Assert.equal(dummy, 3);
});

Tests.register("Console.methods.Console.warn.0", function( ) {
	var dummy = 4;
		console.warn( 'Nidium, Improving the web' );

	Assert.equal(dummy, 4);
});

Tests.register("Console.methods.Console.info.0", function( ) {
	var dummy = 5;
		console.info( 'Nidium, A new breed of browser' );

	Assert.equal(dummy, 5);
});

Tests.register("Console.base.Console.0", function( ) {
	var dummy = 6;
		console.log( 'Nidium' );

	Assert.equal(dummy, 6);
});

Tests.register("System.methods.System.getOpenFileStats.0", function( ) {
	var dummy = 7;
		console.log( JSON.stringify( System.getOpenFileStats() ));

	Assert.equal(dummy, 7);
});

