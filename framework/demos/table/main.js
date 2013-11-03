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


var data = [
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
	{ name : "Dream Theater",	score : 427, progress : "23%" },
	{ name : "Toto",			score : 231, progress : "48%" },
	{ name : "Myrath",			score : 768, progress : "95%" },
	{ name : "SymphonyX",		score : 573, progress : "19%" },
];


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
		id : "cx_"+i,
		label : "#"+i
	});

	for (var key in row){
		var value = row[key];

		r.addCell({
			id : "cx_"+key,
			label : value
		});
	}

}

table.body.rows[3].disabled = true;

var cell_6x1 = table.body.rows[6].cells[1];
cell_6x1.label = "";
cell_6x1.add("UISliderController").move(0, 8);