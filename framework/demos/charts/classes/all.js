/* --------------------------------------------------------------------------- *
 * CHARTJS DEMO - NATiVE PORT                              (c) 2013 nidium.com * 
 * --------------------------------------------------------------------------- * 
 * Original:    http://chartjs.org/                                            *
 * --------------------------------------------------------------------------- * 
 * Released under the MIT license                                              *
 * https://github.com/nnnick/Chart.js/blob/master/LICENSE.md                   *
 * --------------------------------------------------------------------------- * 
 */

var body = new Application({
	class : "body"
});


var	v1 = new UIElement(body, "views").place(10, 10);
var	v2 = new UIElement(body, "views").place(350, 10);
var	v3 = new UIElement(body, "views").place(690, 10);
var	v4 = new UIElement(body, "views").place(10, 280);
var	v5 = new UIElement(body, "views").place(350, 280);
var	v6 = new UIView(body, "views").place(690, 280);

v6.backgroundImage = "assets/grey.png";

var lineChartData = {
	labels : ["January","February","March","April","May","June","July"],
	datasets : [
		{
			fillColor : "rgba(220,220,220,0.5)",
			strokeColor : "rgba(220,220,220,1)",
			pointColor : "rgba(220,220,220,1)",
			pointStrokeColor : "#fff",
			data : [65,59,90,81,56,55,40]
		},
		{
			fillColor : "rgba(151,187,205,0.5)",
			strokeColor : "rgba(151,187,205,1)",
			pointColor : "rgba(151,187,205,1)",
			pointStrokeColor : "#fff",
			data : [28,48,40,19,96,27,100]
		}
	]

};

var pieData = [
	{
		value : 30,
		color :"#F38630"
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

var polarData = [
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

var barChartData = {
	labels : ["January","February","March","April","May","June","July"],
	datasets : [
		{
			fillColor : "rgba(220,220,220,0.5)",
			strokeColor : "rgba(220,220,220,1)",
			data : [65,59,90,81,56,55,40]
		},
		{
			fillColor : "rgba(151,187,205,0.5)",
			strokeColor : "rgba(151,187,205,1)",
			data : [28,48,40,19,96,27,100]
		}
	]
};

var doughnutData = [
	{
		value : 30,
		color :"#F7464A"
	},
	{
		value : 50,
		color : "#46BFBD"
	},
	{
		value : 100,
		color : "#FDB45C"
	},
	{
		value : 40,
		color : "#949FB1"
	},
	{
		value : 120,
		color : "#4D5360"
	}

];

var radarChartData = {
	labels : ["Eating","Drinking","Sleeping","Designing","Coding","Partying","Running"],
	datasets : [
		{
			fillColor : "rgba(220,220,220,0.5)",
			strokeColor : "rgba(220,220,220,1)",
			pointColor : "rgba(220,220,220,1)",
			pointStrokeColor : "#fff",
			data : [65,59,90,81,56,55,40]
		},
		{
			fillColor : "rgba(151,187,205,0.5)",
			strokeColor : "rgba(151,187,205,1)",
			pointColor : "rgba(151,187,205,1)",
			pointStrokeColor : "#fff",
			data : [28,48,40,19,96,27,100]
		}
	]

};

var myLine = 	new Chart(v1).Line(lineChartData);
var myPie = 	new Chart(v2).Pie(pieData);
var myBars = 	new Chart(v3).Bar(barChartData);
var myDonut =	new Chart(v4).Doughnut(doughnutData);
var myPolar =	new Chart(v5).PolarArea(polarData);
var myRadar =	new Chart(v6).Radar(radarChartData, {
	scaleShowLabels : false,
	pointLabelFontSize : 10
});


