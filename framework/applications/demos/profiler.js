/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
load("applications/NatProfiler.nap");
/* -------------------------------------------------------------------------- */


var main = new Application({background:"#262722"});

/* add some dummy time consuming methods to our main object */

main.dummy = function(){
	var m = 5;
	for (var k=0; k<500; k++){
		m = k*2;
	}
};

main.myNewDummy = function lisaDummy(){
	var m = 2;
	for (var k=0; k<500000000; k++){
		m = k*2;
	}
};


/* ---------------------------------------------------------------------- */
/* overwrite main and wrap it using a secure identity-preserving membrane */
/* ---------------------------------------------------------------------- */
var main = Native.profiler.wrap(main, 5000);
/* ---------------------------------------------------------------------- */

/* run 200 times the dummy method */

for (var i=0; i<2000; i++){
	main.dummy();
}

main.myNewDummy();



var	button = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});


var v = [],
	x = y = 0;
for (var i=0; i<256; i++){
	v[i] = new UIButton(main, {
		left : 10 + 34*x,
		top : 48 + 22*y,
		label : i<100 ? (i<10 ? '00'+i : '0'+i) : i,
		width : 30,
		height : 20,
		fontSize : 10,
		background : "rgb("
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)
		+")",
		radius : 6
	});
	v[i].initialLeft = v[i].left;

	x++;
	if (x>15) {
		x = 0;
		y++;
	}
}

button.addEventListener("mousedown", function(e){
	start();
});

var timers = [];

function useNativeSetInterval(element, duration){
	element.duration = duration;
	element.time = 0;
	element.start = element.left;
	element.end = 250;

	clearInterval(element.timer);

	element.timer = setInterval(function(){
		
		element.left = Math.round(
			Math.physics.elasticOut(
			0, element.time, element.start, element.end, element.duration
		));

		element.time += 10;

		if (element.time > element.duration) {
			clearInterval(element.timer);
		}

	}, 10);

}

function useNativeMotionFactory(element, duration){
	element.animate(
		"left",
		element.left, element.left+250,
		duration,
		null,
		Math.physics.elasticOut
	);
}

function start(){
	var k = 1;
	for (var i in v){
		var element = v[i];
		element.left = element.initialLeft;
		
		//useNativeSetInterval(element, 580+1.0*k++);
		useNativeMotionFactory(element, 580+1.0*k++);
	}
}

