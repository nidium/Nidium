/* --------------------------------------------------------------------------- *
 * CHARTJS DEMO                                            (c) 2013 nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Original:    http://chartjs.org/                                            *
 * --------------------------------------------------------------------------- * 
 * Released under the MIT license                                              *
 * https://github.com/nnnick/Chart.js/blob/master/LICENSE.md                   *
 * --------------------------------------------------------------------------- * 
 */

var body = new Application({
	background : "white"
});

var	view = new UIElement(body, {
	id : "view",
	width : 960,
	height : 660
}).center();

var barChartData = {
	labels : ["January","February","March","April","May","June","July"],
	datasets : [
		{
			fillColor : "rgba(220, 220, 220, 0.5)",
			strokeColor : "rgba(220, 220, 220, 1)",
			data : [65,59,90,81,56,55,40]
		},
		{
			fillColor : "rgba(151, 187, 205, 0.5)",
			strokeColor : "rgba(151, 187, 205, 1)",
			data : [28,48,40,19,96,27,100],
			radius : 8
		},
		{
			fillColor : "rgba(225, 127, 75, 0.5)",
			strokeColor : "rgba(225, 127, 75, 1)",
			data : [18,38,20,09,26,17,50]
		}
	]

}

var myLine = new Chart(view).Bar(barChartData);
