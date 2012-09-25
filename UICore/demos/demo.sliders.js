/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */


var main = new Application({background:"#262722"}),

	slider1 = main.add("UISliderController", {
		x : 350,
		y : 150,
		background : '#161712',
		color : 'rgba(210, 255, 40, 1)',
		progressBar : true,
		disabled : false,
		radius : 2,
		min : -100,
		max : 100,
		value : 0
	}),

	slider2 = main.add("UISliderController", {
		x : 350,
		y : 180,
		background : '#161712',
		color : 'rgba(255, 40, 210, 1)',
		progressBar : true,
		disabled : false,
		radius : 2,
		min : -100,
		max : 100,
		value : 0
	}),

	slider3 = main.add("UISliderController", {
		x : 350,
		y : 210,
		w : 250,
		background : '#161712',
		progressBar : false,
		disabled : false,
		radius : 2,
		min : -1,
		max : 1,
		value : 0
	});


slider3.addEventListener("change", function(value){
	var g1 = slider1.value,
		g2 = slider2.value;

	slider1.setValue(g1+value*20);
	slider2.setValue(g2+value*20);
	frequency = (value+1)/5;
}, false);


var sliders = [],
	nb_sliders = 15
	frequency = 0.5;

for (var s=0; s<nb_sliders; s++){
	sliders[s] = main.add("UISliderController", {
		x : 80,
		y : 100 + s*20,
		w : 120,
		h : 10,
		background : '#161712',
		color : '#111111',
		progressBar : false,
		radius : 2,
		min : -20,
		max : 20,
		value : 0
	});
}


setTimeout(function(){
	slider1.setValue(-50, 400);


	var s = 0,
		time = 0;
		t = setTimer(function(){
			sliders[s].setValue(20*Math.sin(frequency*time++), 300);

			s++;
			if (s >= nb_sliders) {
				s = 0;
			}
		}, 40, true, true);



}, 1500);



