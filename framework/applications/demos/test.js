/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	id : "main",
	background : "#262722"
});

var	button = new UIButton(main, {
	id : "button",
	left : 966,
	top : 8,
	label : "Do It"
});

button.addEventListener("mouseup", function(e){
});


log("-----fffd --------");


var i = new Image();

// OK
//i.src = "http://www.google.fr/images/srpr/logo3w.png";
//i.src = "http://fr.fordesigner.com/imguploads/Image/cjbc/zcool/png20080526/1211810004.png";
//i.src = "http://www.nico2012.fr/wp-content/uploads/2012/02/SDF.jpg";


// KO
//i.src = "http://www.allgraphics123.com/ag/01/14142/14142.gif";
//i.src = "http://www.spambattle.com/wp-content/themes/default/custom/rotator/home-4.jpg";
//i.src = "http://k02.kn3.net/CE987F60E.jpg";

//i.src = "http://s.tf1.fr/mmdia/i/77/6/une-plage-image-d-illustration-10737776ojmko_1713.jpg?v=1";

i.src = "http://www.nico2012.fr/wp-content/uploads/2012/02/SDF.jpg";

i.onload = function(){
	echo("loaded");
	n = new Canvas(400, 400);
	Native.canvas.add(n);

	//n.ctx.fillStyle = n.ctx.createPattern(i, "repeat");
	//n.ctx.fillRect(0, 0, 400, 400);
	n.ctx.drawImage(i, 0, 0);
};










