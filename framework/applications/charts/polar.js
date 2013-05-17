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

document.background = "white";

var body = new Application({
	backgroundImage : "falcon/assets/back.png",
	alpha : 0.75
});

var	view = new UIElement(body, {
	width : 650,
	height : 530
}).center();

var chartData = [
	{
		value : Math.random(),
		color : "#D97041"
	},
	{
		value : Math.random(),
		color : "#C7604C"
	},
	{
		value : Math.random(),
		color : "#21323D"
	},
	{
		value : Math.random(),
		color : "#9D9B7F"
	},
	{
		value : Math.random(),
		color : "#7D4F6D"
	},
	{
		value : Math.random(),
		color : "#584A5E"
	}
];

var myPolarArea = new Chart(view).PolarArea(chartData);

