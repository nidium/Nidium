/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"});

var LFO = main.add("UIDiagram", {
	x : 180, 
	y : 320, 
	label : "Low Frequency Oscillator",
	background : "#222222",
	elements : 	[
		{label:"Input",				type:"output"},
		{label:"Cutoff Frequency",	type:"output"},
		{label:"Resonnance",		type:"output"},
		{label:"Output",			type:"output"}
	]
});

var LPF = main.add("UIDiagram", {
	x : 600, 
	y : 480, 
	label : "Low Pass Filter",
	background : "#222222",
	elements : 	[
		{label:"Input",				type:"input"},
		{label:"Cutoff Frequency",	type:"controller"},
		{label:"Resonnance",		type:"controller"},
		{label:"Output",			type:"output"}
	]
});

var VCA = main.add("UIDiagram", {
	x : 750, 
	y : 80, 
	label : "Voltage Controlled Amplifier",
	background : "#331111",
	elements : 	[
		{label:"In L",	type:"input"},
		{label:"In R",	type:"input"},
		{label:"Out L",	type:"output"},
		{label:"Out R",	type:"output"}
	]
});

DBT(function(){
	LPF.remove();
});


/*

var link = main.add("UILine", {
	vertices : [
		10, 600,
		100, 500,
		200, 700,
		300, 500,
		400, 700
	],
	displayControlPoints : true,
	color : "#ff0000",
	lineWidth : 3
});

*/
