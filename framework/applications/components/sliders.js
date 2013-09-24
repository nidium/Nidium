/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

var t = false;

button.addEventListener("mouseup", function(e){
	t = !t;
	slider3.vertical = t;
});

// Small Green Slider
var slider1 = main.add("UISliderController", {
	left : 350,
	top : 150,
	background : '#161712',
	color : 'rgba(210, 255, 40, 1)',
	progressBarColor : 'rgba(210, 255, 40, 1)',
	disabled : false,
	radius : 2,
	min : -100,
	max : 100,
	value : 0
}),

// Small Rose Slider
slider2 = main.add("UISliderController", {
	left : 350,
	top : 180,
	background : '#161712',
	color : 'rgba(255, 40, 210, 1)',
	progressBarColor : 'rgba(255, 40, 210, 1)',
	disabled : false,
	radius : 2,
	min : -100,
	max : 100,
	value : 0
}),

// Big Blue Slider
slider3 = main.add("UISliderController", {
	left : 350,
	top : 240,
	width : 250,
	height : 20,

	displayLabel : true,
	labelBackground : 'rgba(255, 255, 255, 0.7)',
	labelColor : 'rgba(0, 0, 0, 0.5)',
	labelOffset : -18,
	labelWidth : 56,
	labelPrefix : 'f=',
	labelSuffix : ' %',
	fontSize : 10,
	lineHeight : 18,

	splitColor : 'rgba(0, 0, 0, 0.5)',
	boxColor : 'rgba(255, 255, 255, 0.02)',
	
	disabled : false,
	radius : 2,
	min : -1,
	max : 1,
	value : 0
}),

master = main.add("UISliderController", {
	left : 800,
	top : 210,
	width : 12,
	height : 300,
	vertical : true,
	background : '#161712',
	color : 'rgba(255, 40, 210, 1)',
	progressBarColor : 'rgba(255, 40, 210, 1)',
	splitColor : 'rgba(50, 40, 10, 0.7)',
	disabled : false,
	radius : 3,
	min : 0,
	max : 300,
	value : 0
});


master.addEventListener("complete", function(value){
	console.log(value);
}, false);



// Big Blue Slider controls the small sliders
slider3.addEventListener("change", function(e){
	var g1 = slider1.value,
		g2 = slider2.value;

	slider1.setValue(g1+e.value*20);
	slider2.setValue(g2+e.value*20);
	frequency = (e.value)/10;
}, false);



/*
 * -- Sine Sliders -----------------
 */



var sliders = [],
	nb_sliders = 32,
	frequency = 4.8,
	sliderTop = 12,
	sliderLeft = 16,
	sliderWidth = 140,
	gdSpectrum = window.canvas.ctx.createLinearGradient(0, 0, sliderWidth, 0);

gdSpectrum.addColorStop(0.00,'#002200');
gdSpectrum.addColorStop(0.25,'#00ff00');
gdSpectrum.addColorStop(0.50,'#ffff00');
gdSpectrum.addColorStop(1.00,'#ff0000');

for (var s=0; s<nb_sliders; s++){
	sliders[s] = main.add("UISliderController", {
		left : sliderLeft,
		top : sliderTop + s*23,
		width : sliderWidth,
		height : 18,
		background : '#161712',
		color : '#111111',
		boxColor : 'rgba(0, 0, 0, 1)',
		progressBarColor : gdSpectrum,
		splitColor : 'rgba(60, 60, 60, 0.40)',
		ease : Math.physics.cubicIn,
		radius : 3,
		min : -20,
		max : 20,
		value : 0
	});
}

setTimeout(function(){
	slider1.setValue(-50, 400);

	var s = 0,
		time = 0,
		t = window.timer(function(){
			sliders[s].setValue(20*Math.cos(3*frequency*time++), 300);

			s++;
			if (s >= nb_sliders) {
				s = 0;
			}
		}, 25, true, true);



}, 200);

