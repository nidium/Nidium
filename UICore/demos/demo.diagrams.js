/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"rgba(30, 20, 20, 0.7)", x:40, y:40});

var myDiagram = main.add("UIDiagramController", {x:30, y:90});


var LFO = myDiagram.add("UIDiagram", {
	x : 180, 
	y : 320,
	label : "Low Frequency Oscillator",
	background : "#222222",
	pins : [
	/* pin 0 */	{label:"Pitch",			type:"output"},
	/* pin 1 */	{label:"Frequency",		type:"output"},
	/* pin 2 */	{label:"Resonnance",	type:"output"},
	/* pin 3 */	{label:"Audio Out",		type:"output"}
	]
});

var LPF = myDiagram.add("UIDiagram", {
	x : 600, 
	y : 480, 
	label : "Low Pass Filter",
	background : "#224422",
	pins : [
	/* pin 0 */	{label:"Input",				type:"input"},
	/* pin 1 */	{label:"Cutoff Frequency",	type:"controller"},
	/* pin 2 */	{label:"Resonnance",		type:"controller"},
	/* pin 3 */	{label:"Output",			type:"output"}
	]
});

var VCA = myDiagram.add("UIDiagram", {
	x : 750, 
	y : 80, 
	label : "Voltage Controlled Amplifier",
	background : "#331111",
	pins : [
	/* pin 0 */	{label:"In L",	type:"input"},
	/* pin 1 */	{label:"In R",	type:"input"},
	/* pin 2 */	{label:"Out L",	type:"output"},
	/* pin 3 */	{label:"Out R",	type:"output"}
	]
});

/*
myDiagram.link({

});
*/

main.addEventListener("load", function(){
	myDiagram.connect(LFO.pins[1], VCA.pins[0]);
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
myDiagram.addEventListener("pinEnter", function(link){
}, false);

myDiagram.addEventListener("pinOver", function(link){
	link.source.pin.color = '#009900';
	link.target.pin.background = '#ff9900';
}, false);

myDiagram.addEventListener("pinLeave", function(link){
	link.source.pin.color = '';
	link.target.pin.background = '';
}, false);

myDiagram.addEventListener("pinDrop", function(link){
	var s = link.source,
		t = link.target;

	echo("[" + s.label + "] " + s.pin.label + "-" + t.pin.label + " [" + t.label + "]");
	s.pin.color = '';
	t.pin.background = '';
}, false);



