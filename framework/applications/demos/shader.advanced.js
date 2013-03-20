/* -------------------------------------- */
/* High Level Shader API (advanced demo)  */
/* -------------------------------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	width : 1004,
	height : 680
}).center();
		
var ShaderDemo = {
	init : function(){
		var self = this;

		main.shader("applications/demos/shaders/apple.s", function(p, u){
			self.start(p, u);
		});
	},

	start : function(program, uniforms){
		var t = 0;

		this.createSlider(uniforms);

		setInterval(function(){
			uniforms.itime = t++;
		}, 16);
	},

	createSlider : function(uniforms){
		var self = this;

		this.slider1 = main.add("UISliderController", {
			left : 10,
			top : 20,
			width : 180,
			background : '#161712',
			color : 'rgba(0, 40, 50, 1)',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : 0,
			max : 100,
			value : 0
		});

		this.slider2 = main.add("UISliderController", {
			left : 10,
			top : 45,
			width : 180,
			background : '#161712',
			color : 'rgba(0, 40, 50, 1)',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : 0,
			max : 200,
			value : 0
		});

		this.slider3 = main.add("UISliderController", {
			left : 10,
			top : 70,
			width : 180,
			background : '#161712',
			color : 'rgba(0, 40, 50, 1)',
			progressBarColor : 'rgba(255, 255, 255, 1)',
			disabled : false,
			radius : 2,
			min : -100,
			max : 200,
			value : 0
		});

		this.slider1.addEventListener("update", function(value){
			uniforms.data = value;
		}, false);

		this.slider2.addEventListener("update", function(value){
			uniforms.param = value;
		}, false);

		this.slider3.addEventListener("update", function(value){
			uniforms.zoom = value;
		}, false);

		document.addEventListener("mousewheel", function(e){
			var gg = Math.round(uniforms.zoom + e.yrel*2);
			gg = Math.min(gg, 200);
			gg = Math.max(gg, -100);
			self.slider3.setValue(gg);
		});

	}

};

ShaderDemo.init();

