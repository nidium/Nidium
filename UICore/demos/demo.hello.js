
var main = new Application({background:"#222222"}),
	stephen = main.add("UIButton", {x:80, y:110, label:"Stephen", background:"#000000", radius:4}),

	radio1 = main.add("UIRadio", {x:5, y:36, name:"choice", label:"Selectionnez ce choix", selected:true}),
	radio2 = main.add("UIRadio", {x:5, y:56, name:"choice", label:"Ou alors celui la"}),

	line1 = main.add("UILine", {x1:200, y1:200, x2:500, y2:200, color:"#ff0000", quadratic:true}),
	line2 = main.add("UILine", {x1:40, y1:150, x2:400, y2:327, color:"#ffff00"});


stephen.addEventListener("mousedown", function(){
	line1.x1 = line1.x1 + 10;
});
