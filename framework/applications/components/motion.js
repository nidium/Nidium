/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application();

var	button = new UIButton(main, {
	left : 966,
	top : 8,
	label : "Do It"
});

var tpl = {
	left : 30,
	top : 30,
	width : 32,
	height : 32,
	background : "rgba(50, 50, 190, 0.8)",
	radius : 20
};

var	o1 = new UIElement(main, tpl);
var	o2 = new UIElement(main, tpl).move(40, 0);
var	o3 = new UIElement(main, tpl).move(80, 0);;
var	o4 = new UIElement(main, tpl).move(120, 0);;

button.addEventListener("mousedown", function(e){


});



var Animation = function(na){
	for (var k in na){
		console.log(k);
	}
};





var k = new Animation({
	"foobar" : {
		left : 50,
		top : 150
	},

	"#mmm" : {
		top : {
			from : 20,
			to : 80,
			time : 500,
			loop : true,
			reverse : true,
			ease : Math.physics.bounceOut
		}
	},
	
	"#c0.top" : 80,

	"#bar.width, .view.top, .c0.left" : ,

	"#sphere" : {
		time : 250,
		position : function(t){
			var x = 0,
				y = 0;

			return [x, y];
		}
	}
});

//UIElement.follow(path);

