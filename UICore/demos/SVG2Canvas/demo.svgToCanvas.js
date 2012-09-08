/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/* --- DEMO APP --------------------------------------------------------- */


// http://www.professorcloud.com/svg-to-canvas/


var document = new Application({animation:false});
var ctx = canvas;

canvas.requestAnimationFrame(function(){
	ctx.save();
	ctx.beginPath();
	ctx.moveTo(0,0);
	ctx.lineTo(600,0);
	ctx.lineTo(600,660);
	ctx.lineTo(0,660);
	ctx.closePath();
	ctx.clip();
	ctx.strokeStyle = 'rgba(0,0,0,0)';
	ctx.lineCap = 'butt';
	ctx.lineJoin = 'miter';
	ctx.miterLimit = 4;
	ctx.save();
	ctx.restore();
	ctx.save();
	ctx.save();
	ctx.fillStyle = "#ffffff";
	ctx.strokeStyle = "rgba(0, 0, 0, 0)";
	ctx.lineWidth = 1;
	ctx.lineCap = "butt";
	ctx.lineJoin = "miter";
	ctx.beginPath();
	ctx.moveTo(300,658.5);
	ctx.bezierCurveTo(300,658.5,598.50001,546.17969,598.50001,260.72817);
	ctx.bezierCurveTo(598.50001,-24.723354,598.50001,2.1764765,598.50001,2.1764765);
	ctx.lineTo(1.5,2.1764765);
	ctx.lineTo(1.5,260.72817);
	ctx.bezierCurveTo(1.5,546.17969,300,658.5,300,658.5);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();
	ctx.restore();
	ctx.save();
	ctx.fillStyle = "#e20909";
	ctx.strokeStyle = "rgba(0, 0, 0, 0)";
	ctx.lineWidth = 1;
	ctx.lineCap = "butt";
	ctx.lineJoin = "miter";
	ctx.beginPath();
	ctx.moveTo(1.4664755,127.20164);
	ctx.lineTo(452.70392,563.75532);
	ctx.bezierCurveTo(490.13282,536.79144,546.32698,460.33371,562.32086,422.00782);
	ctx.lineTo(129.39267,1.9608375);
	ctx.lineTo(2.0902848,3.3354451);
	ctx.lineTo(1.4664755,127.20164);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();
	ctx.save();
	ctx.save();
	ctx.fillStyle = "rgba(0, 0, 0, 0)";
	ctx.strokeStyle = "#000000";
	ctx.lineWidth = 3.000000476837158;
	ctx.lineCap = "butt";
	ctx.lineJoin = "miter";
	ctx.miterLimit = 4;
	ctx.globalAlpha = 1;
	ctx.beginPath();
	ctx.moveTo(299.99999,658.50028);
	ctx.bezierCurveTo(299.99999,658.50028,1.5,546.17992,1.5,260.72828);
	ctx.bezierCurveTo(1.5,-24.723336,1.5,2.1764957,1.5,2.1764957);
	ctx.lineTo(598.49999,2.1764957);
	ctx.lineTo(598.49999,260.72828);
	ctx.bezierCurveTo(598.49999,546.17992,299.99999,658.50028,299.99999,658.50028);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();
	ctx.restore();
	ctx.save();
	ctx.translate(-995.12739,57.552536);
	ctx.restore();
	ctx.save();
	ctx.translate(0.029768821,0.1606275);
	ctx.save();
	g=ctx.createRadialGradient(222.03383409166003,219.55730665727992,0,222.03383409166003,219.55730665727992,300);
	g.addColorStop(0,"rgba(255, 255, 255, 0.3137255)");
	g.addColorStop(0.19,"rgba(255, 255, 255, 0.25098041)");
	g.addColorStop(0.60000002,"rgba(107, 107, 107, 0.1254902)");
	g.addColorStop(1,"rgba(0, 0, 0, 0.1254902)");
	ctx.fillStyle = g;
	ctx.strokeStyle = "rgba(0, 0, 0, 0)";
	ctx.lineWidth = 1;
	ctx.lineCap = "butt";
	ctx.lineJoin = "miter";
	ctx.globalAlpha = 1;
	ctx.beginPath();
	ctx.moveTo(300.00001,658.5);
	ctx.bezierCurveTo(300.00001,658.5,598.50001,546.17969,598.50001,260.72817);
	ctx.bezierCurveTo(598.50001,-24.723334,598.50001,2.1764865,598.50001,2.1764865);
	ctx.lineTo(1.5,2.1764865);
	ctx.lineTo(1.5,260.72817);
	ctx.bezierCurveTo(1.5,546.17969,300.00001,658.5,300.00001,658.5);
	ctx.closePath();
	ctx.fill();
	ctx.stroke();
	ctx.restore();
	ctx.restore();
	ctx.restore();
});



