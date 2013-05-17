/* --------------------------------------------------------------------------- *
 * CHARTJS DEMO                                            (c) 2013 Stight.com * 
 * --------------------------------------------------------------------------- * 
 * Original:    http://chartjs.org/                                            *
 * --------------------------------------------------------------------------- * 
 * Released under the MIT license                                              *
 * https://github.com/nnnick/Chart.js/blob/master/LICENSE.md                   *
 * --------------------------------------------------------------------------- * 
 */

/* -------------------------------------------------------------------------- */
include("libs/chart.lib.js");
/* -------------------------------------------------------------------------- */

var body = new Application({
	background : "white"
});

var	view = new UIElement(body, {
	left : 30,
	top : 30,
	width : 500,
	height : 500,
	background : "rgba(255, 255, 255, 1.0)",
	shadowBlur : 14,
	shadowColor : "rgba(0, 0, 0, 0.2)",
	radius : 12
});

var radarChartData = {
	labels : ["Eating", "Drinking", "Sleeping", "Designing", "Coding", "Partying", "Running"],
	datasets : [
		{
			fillColor : "rgba(220, 220, 220, 0.5)",
			strokeColor : "rgba(220, 220, 220, 1)",
			pointColor : "rgba(220, 220, 220, 1)",
			pointStrokeColor : "#fff",
			data : [65,59,90,81,56,55,40]
		},
		{
			fillColor : "rgba(151, 187, 205, 0.5)",
			strokeColor : "rgba(151, 187, 205, 1)",
			pointColor : "rgba(151, 187, 205, 1)",
			pointStrokeColor : "#fff",
			data : [28,48,40,19,96,27,100]
		}
	]

};

var myRadar = new Chart(view).Radar(radarChartData, {
	scaleShowLabels : false,
	pointLabelFontSize : 10
});


