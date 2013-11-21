/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

document.backgroundImage = "private://assets/patterns/egg_shell.png";

document.nss.add({
	"UITableView" : {
		width : 1010,
		height : 750
	},

	"UITableHead" : {
		height : 28,
		fontSize : 11,
		color : "white",
		fontFamily : "menlo",
		background : "rgba(0, 0, 0, 0.6)",
	},

	"UITableRow" : {
		height : 28,
		background : "rgba(255, 255, 255, 0.5)"
	},

	"UITableRow:even" : {
		background : "rgba(220, 240, 255, 0.6)"
	},

	"UITableRow:selected" : {
		color : "#ffffff",
		background : "#2277E0"
	}

});

var table = new UITableView(document).center();
table.head.fixed = true;

table.setHead([
	{
		label : "Identifier",
		width : 120,
		background : null,
		color : "white"
	},
	{
		label : "Name",
		width : 350
	},
	{
		label : "Score",
		width : 350
	},
	{
		label : "Progress",
		width : 450
	}
]);


var feed = function(data){
	for (var i=0; i<data.length; i++){
		var row = data[i];

		var r = table.addRow({
			id : "rx"+i
		});

		r.click(function(e){
			if (this.disabled) return false;
			this.selected = !this.selected;
		});

		r.addCell({
			id : "cx"+i,
			label : "#"+i
		});

		for (var key in row){
			var value = row[key];

			r.addCell({
				id : "cx"+key,
				label : value
			});
		}

	}

	table.body.rows[3].disabled = true;
	table.invertScrollDirection();
	table.invertScrollDirection();
	table.spacedrag = true;

	var r6 = table.body.rows[6],
		cell_6x1 = r6.cells[1];

	cell_6x1.label = "";
	var g = cell_6x1.add("UISliderController").move(0, 8);
	g.value = 50;
	g.height = 30;



	g.style.toto = 5;
	console.log(g.style.toto);

	/*
	r6.left = 40;
	r6.top = 0;
	r6.background = "red";
	*/

	/*

	var m = r6.cloneNode(true);
	console.log(m.id, "rooted:", m.rooted, "parent:", m.parent ? m.parent.id : null);

	m.move(0, -60);
	m.background = "red";

	for (var i=0; i<m.nodes.length; i++) {
		console.log(m.nodes[i].type, m.nodes[i].id, m.nodes[i].parent.id);
	}

	console.log("-------------------");
	var p = table.body.rows[6].cells[0];
	for (var i in p) {
		//console.log(i, ":", typeof p[i]);
	}

	NatBug.init();
	*/

};

File.getText("data.json", function(text){
	var data = JSON.parse(text);
	feed(data);
});
