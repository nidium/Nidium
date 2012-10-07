/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"});

var LFO = main.add("UIDiagram", {
	x : 180, 
	y : 320,
	label : "Low Frequency Oscillator",
	background : "#222222",
	pins : [
		{label:"Pitch",			type:"output"},
		{label:"Frequency",		type:"output"},
		{label:"Resonnance",	type:"output"},
		{label:"Audio Out",		type:"output"}
	]
});

var LPF = main.add("UIDiagram", {
	x : 600, 
	y : 480, 
	label : "Low Pass Filter",
	background : "#224422",
	pins : [
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
	pins : [
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
 *	link.source : Source UIDiagram Element (source diagram)
 *	link.source.pin : Source UILabel Element (source pin of the drag operation)
 * 
 *	link.target : Target UIDiagram Element (target diagram) 
 *	link.target.pin : Target UILabel Element (target pin of the drag operation)
 *
 */
main.addEventListener("pinEnter", function(link){
}, false);

main.addEventListener("pinOver", function(link){
	link.source.pin.color = '#009900';
	link.target.pin.background = '#ff9900';
}, false);

main.addEventListener("pinLeave", function(link){
	link.source.pin.color = '';
	link.target.pin.background = '';
}, false);

main.addEventListener("pinDrop", function(link){
	var s = link.source,
		t = link.target;

	echo("[" + s.label + "] " + s.pin.label + "-" + t.pin.label + " [" + t.label + "]");
	s.pin.color = '';
	t.pin.background = '';
}, false);



