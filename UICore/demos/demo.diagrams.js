/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"rgba(30, 20, 20, 0.7)", x:-40, y:-40});

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
	myDiagram.connect(VCA.pins[0], LFO.pins[1]);
});


DBT(function(){
	LPF.remove();
	myDiagram.connect(LFO.pins[1], VCA.pins[0]);
});


/*
 *  UIDiagramController Events
 *  --------------------------
 *	e.source : Source UIDiagram Element (source diagram)
 *	e.source.pin : Source UILabel Element (source pin of the drag operation)
 * 
 *	e.target : Target UIDiagram Element (target diagram) 
 *	e.target.pin : Target UILabel Element (target pin of the drag operation)
 *
 */
myDiagram.addEventListener("pinEnter", function(e){
}, false);

myDiagram.addEventListener("pinOver", function(e){
	e.source.pin.color = '#009900';
	e.target.pin.background = '#ff9900';
}, false);

myDiagram.addEventListener("pinLeave", function(e){
	e.source.pin.color = '';
	e.target.pin.background = '';
}, false);

myDiagram.addEventListener("pinDrop", function(e){
	var s = e.source,
		t = e.target;

	echo("[" + s.label + "] " + s.pin.label + "-" + t.pin.label + " [" + t.label + "]");
	s.pin.color = '';
	t.pin.background = '';
}, false);

myDiagram.addEventListener("alreadyconnected", function(e){
	echo("already connected:", e.source.pin.label+ "-" + e.target.pin.label);
}, false);


myDiagram.addEventListener("connect", function(e){
	echo("connect:", e.source.pin.label+ "-" + e.target.pin.label);
	//e.refuse();
}, false);



