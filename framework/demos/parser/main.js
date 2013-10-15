/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/*
 
    JS <--> JS demo using AST as intermediate representation
 
   	+----+          +-----+           +---------+             +---------+
   	|    |  PARSE   |     |  EXPORT   |         |  BEAUTIFY   |         |
   	| JS | -------> | AST | --------> | UGLY JS | ----------> | NICE JS |
   	|    |          |     |           |         |             |         |
   	+----+          +-----+           +---------+             +---------+
 
*/

var file = "jquery.js";

function job(ast, callback){

	var t = new Thread(function(ast, scope){
		var source = scope.generate(ast);
		return source;
	});

	t.onmessage = function(e){
		console.log(e.data);
	};

	t.oncomplete = function(e){
		if (typeof(callback)=="function") callback(e.data);
	};

	t.start(ast, {
		generate : function(ast){ return "lolo" },
		beauty : window.code.beautify
	});

};

//console.log(window.code.export);


File.getText("/source/"+file, function(text){
	var time,
		ast = {},
		source = "";

	time = +new Date();
	ast = window.code.parse(text);
	console.log("Parsing was done in", +new Date() - time, "ms");

	job(ast, function(source){
		console.log(source);
	});


	/*
	time = +new Date();
	source = window.code.export(ast);
	console.log("Transpilation was done in", +new Date() - time, "ms");

	time = +new Date();
	source = window.code.beautify(source);
	console.log("Beautified in", +new Date() - time, "ms");

	File.write("/export/"+file, source);
	*/

});
