/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"});

var LPF = main.add("UIDiagram", {
	x : 180, 
	y : 320, 
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
	x : 500, 
	y : 160, 
	label : "Voltage Controlled Amplifier",
	background : "#331111",
	elements : 	[
		{label:"In L",	type:"input"},
		{label:"In R",	type:"input"},
		{label:"Out L",	type:"output"},
		{label:"Out R",	type:"output"}
	]
});

var link = main.add("UILine", {
	path : [
		50, 150,
		200, 70,
		340, 90,
		460, 200,
		500, 400
	],
	controlPoints : true,
	color : "#ff0000",
	lineWidth : 5
});


