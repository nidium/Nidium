/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

__DEBUG_SHOW_LAYERS__ = true;

var main = new Application({
	backgroundImage : "private://assets/patterns/retina_wood.png"
});

var	button = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});

var v = [],
	x = y = 0;
for (var i=0; i<256; i++){
	v[i] = new UIButton(main, {
		left : 10 + 38*x,
		top : 48 + 42*y,
		label : i<100 ? (i<10 ? '00'+i : '0'+i) : i,
		width : 30,
		height : 38,
		fontSize : 10,
		background : "rgb("
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)+", "
			+Math.round(Math.random()*180)
		+")",
		opacity : 0.9,
		radius : 18
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
		useNativeSetInterval(element, 580+2.91*k++);
		//useNativeMotionFactory(element, 580+0.0*k++);
	}
}

