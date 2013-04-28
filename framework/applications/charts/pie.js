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
require("libs/chart.lib.js");
/* -------------------------------------------------------------------------- */

var body = new Application({
	backgroundImage : "falcon/assets/back.png"
});

var	view = new UIElement(body, {
	width : 650,
	height : 530,
	background : "white",
	shadowBlur : 10,
	shadowColor : "white",
	radius : 16
}).center();

var pieData = [
	{
		value: 30,
		color:"#F38630"
	},
	{
		value : 50,
		color : "#E0E4CC"
	},
	{
		value : 100,
		color : "#69D2E7"
	}

];

var myLine = new Chart(view).Pie(pieData);

