/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP --------------------------------------------------------- */


var document = new Application({animation:false});
var ctx = canvas;

canvas.requestAnimationFrame(function(){


	draw(150, 150);

});

canvas.onmousemove = function(e){
	canvas.mouseX = e.x;
	canvas.mouseY = e.y;
};


var draw = function(x, y){
	ctx.save();
	ctx.translate(canvas.mouseX, canvas.mouseY);

	ctx.save();
	ctx.beginPath();
	ctx.moveTo(0,0);
	ctx.lineTo(125,0);
	ctx.lineTo(125,125);
	ctx.lineTo(0,125);
	ctx.closePath();
	ctx.clip();
	ctx.strokeStyle = 'rgba(0,0,0,0)';
	ctx.lineCap = 'butt';
	ctx.lineJoin = 'miter';
	ctx.miterLimit = 4;

	ctx.save();
	ctx.fillStyle = "#000000";
	ctx.strokeStyle = "rgba(0, 0, 0, 0)";
	ctx.translate(0,100);
	ctx.scale(0.1,-0.1);

	ctx.save();
	ctx.beginPath();
	ctx.moveTo(400,984);
	ctx.bezierCurveTo(304,964,219,918,151,850);
	ctx.bezierCurveTo(44,743,-13,582,7,445);
	ctx.bezierCurveTo(43,206,206,43,445,7);
	ctx.bezierCurveTo(582,-13,743,44,850,151);
	ctx.bezierCurveTo(922,224,965,308,987,418);
	ctx.bezierCurveTo(1002,493,1002,507,987,582);
	ctx.bezierCurveTo(945,794,802,941,595,985);
	ctx.bezierCurveTo(511,1003,484,1003,400,984);
	ctx.closePath();
	ctx.moveTo(582,919);
	ctx.bezierCurveTo(666,901,738,862,800,800);
	ctx.bezierCurveTo(969,631,969,368,801,199);
	ctx.bezierCurveTo(633,31,367,31,199,199);
	ctx.bezierCurveTo(135,264,96,339,79,433);
	ctx.bezierCurveTo(40,646,186,863,405,916);
	ctx.bezierCurveTo(470,932,516,933,582,919);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();

	ctx.save();
	ctx.beginPath();
	ctx.moveTo(620,785);
	ctx.bezierCurveTo(606,769,614,750,636,750);
	ctx.bezierCurveTo(661,750,651,726,619,708);
	ctx.bezierCurveTo(602,698,578,690,566,690);
	ctx.bezierCurveTo(529,690,480,669,470,650);
	ctx.bezierCurveTo(464,640,460,598,460,557);
	ctx.bezierCurveTo(460,516,457,455,454,421);
	ctx.bezierCurveTo(448,369,444,360,431,366);
	ctx.bezierCurveTo(422,369,390,378,360,385);
	ctx.bezierCurveTo(330,391,288,403,268,410);
	ctx.bezierCurveTo(234,422,229,422,210,405);
	ctx.bezierCurveTo(199,395,190,381,190,374);
	ctx.bezierCurveTo(190,366,214,337,242,307);
	ctx.bezierCurveTo(271,278,303,242,313,227);
	ctx.bezierCurveTo(334,196,402,173,502,163);
	ctx.bezierCurveTo(560,158,570,160,570,173);
	ctx.bezierCurveTo(570,181,564,191,558,193);
	ctx.bezierCurveTo(551,196,559,196,576,193);
	ctx.bezierCurveTo(621,186,672,212,716,265);
	ctx.bezierCurveTo(737,290,771,321,790,333);
	ctx.bezierCurveTo(809,344,828,358,831,363);
	ctx.bezierCurveTo(841,377,830,407,810,425);
	ctx.bezierCurveTo(794,440,788,439,731,420);
	ctx.bezierCurveTo(698,409,659,400,645,400);
	ctx.bezierCurveTo(616,400,530,359,511,337);
	ctx.bezierCurveTo(494,315,514,305,574,305);
	ctx.bezierCurveTo(637,305,643,287,590,260);
	ctx.bezierCurveTo(558,244,539,240,504,246);
	ctx.bezierCurveTo(467,251,460,250,460,236);
	ctx.bezierCurveTo(460,217,458,217,426,231);
	ctx.bezierCurveTo(397,245,385,269,400,284);
	ctx.bezierCurveTo(407,291,424,289,456,277);
	ctx.bezierCurveTo(528,251,545,278,487,327);
	ctx.bezierCurveTo(459,350,455,359,461,380);
	ctx.lineTo(468,405);
	ctx.lineTo(469,378);
	ctx.bezierCurveTo(471,335,490,334,492,377);
	ctx.bezierCurveTo(493,414,493,414,497,383);
	ctx.bezierCurveTo(502,346,520,338,521,373);
	ctx.bezierCurveTo(521,385,524,389,527,382);
	ctx.bezierCurveTo(538,354,546,383,539,427);
	ctx.bezierCurveTo(527,500,529,585,542,598);
	ctx.bezierCurveTo(559,615,607,613,641,593);
	ctx.bezierCurveTo(660,582,673,579,678,585);
	ctx.bezierCurveTo(682,591,695,600,706,607);
	ctx.bezierCurveTo(725,617,731,615,751,594);
	ctx.bezierCurveTo(763,581,777,570,782,570);
	ctx.bezierCurveTo(786,570,790,611,790,660);
	ctx.bezierCurveTo(790,710,786,750,781,750);
	ctx.bezierCurveTo(776,750,763,739,752,725);
	ctx.bezierCurveTo(734,702,729,701,706,711);
	ctx.bezierCurveTo(671,727,673,743,711,757);
	ctx.bezierCurveTo(738,768,741,772,730,785);
	ctx.bezierCurveTo(714,805,637,805,620,785);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();
	ctx.restore();
	ctx.restore();
	ctx.restore();
};



