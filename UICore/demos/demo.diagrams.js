/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"rgba(30, 20, 20, 0.7)", x:0, y:0});

var myDiagram = main.add("UIDiagramController", {x:0, y:0});

var LFO = myDiagram.add("UIDiagram", {
	x : 50, 
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

var LPF1 = myDiagram.add("UIDiagram", {
	x : 320, 
	y : 200, 
	label : "Low Pass Filter",
	background : "#224422",
	pins : [
	/* pin 0 */	{label:"Input",				type:"input"},
	/* pin 1 */	{label:"Cutoff Frequency",	type:"controller"},
	/* pin 2 */	{label:"Resonnance",		type:"controller"},
	/* pin 3 */	{label:"Output",			type:"output"}
	]
});

var LPF2 = myDiagram.add("UIDiagram", {
	x : 320, 
	y : 300, 
	label : "Low Pass Filter",
	background : "#224422",
	pins : [
	/* pin 0 */	{label:"Input",				type:"input"},
	/* pin 1 */	{label:"Cutoff Frequency",	type:"controller"},
	/* pin 2 */	{label:"Resonnance",		type:"controller"},
	/* pin 3 */	{label:"Output",			type:"output"}
	]
});

var MIXER = myDiagram.add("UIDiagram", {
	x : 720, 
	y : 460, 
	label : "Stereo Mixer",
	background : "rgba(50, 20, 160, 0.4)",
	pins : [
	/* pin 0 */	{label:"Overall Gain",	type:"controller"},
	/* pin 1 */	{label:"Source 1 L",	type:"input"},
	/* pin 2 */	{label:"Source 1 R",	type:"input"},
	/* pin 3 */	{label:"Source 2 L",	type:"input"},
	/* pin 4 */	{label:"Source 2 R",	type:"input"},
	/* pin 5 */	{label:"Output L",		type:"output"},
	/* pin 6 */	{label:"Output R",		type:"output"},
	]
});


var VCA = myDiagram.add("UIDiagram", {
	x : 500, 
	y : 290, 
	label : "Voltage Controlled Amplifier",
	background : "#331111",
	pins : [
	/* pin 0 */	{label:"In L",	type:"input"},
	/* pin 1 */	{label:"In R",	type:"input"},
	/* pin 2 */	{label:"Out L",	type:"output"},
	/* pin 3 */	{label:"Out R",	type:"output"}
	]
});


var OUTPUT = myDiagram.add("UIDiagram", {
	x : 880, 
	y : 350, 
	label : "Speakers",
	background : "#111111",
	pins : [
	/* pin 0 */	{label:"Audio L",	type:"input"},
	/* pin 1 */	{label:"Audio R",	type:"input"},
	]
});


main.addEventListener("load", function(){
	myDiagram.connect(LFO.pins[3], LPF1.pins[0]);
	myDiagram.connect(LFO.pins[3], LPF2.pins[0]);

	myDiagram.connect(LFO.pins[1], LPF1.pins[1], {color:"#2288DD", lineWidth:2});
	myDiagram.connect(LFO.pins[1], LPF2.pins[1], {color:"#2288DD", lineWidth:2});


	myDiagram.connect(LPF1.pins[3], VCA.pins[0]);
	myDiagram.connect(LPF2.pins[3], VCA.pins[1]);


	// c
	myDiagram.connect(VCA.pins[2], MIXER.pins[1]);
	myDiagram.connect(VCA.pins[3], MIXER.pins[2]);

	// connect mixer --> speakers
	myDiagram.connect(MIXER.pins[5], OUTPUT.pins[0]);
	myDiagram.connect(MIXER.pins[6], OUTPUT.pins[1]);

});



DBT(function(){
	LPF1.remove();
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
	echo("dd connect:", e.source.pin.label+ "-" + e.target.pin.label);
	//e.refuse();
}, false);




