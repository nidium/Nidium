/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({id:"main"});

var doit = new UIButton(main).click(function(){
	h.disable();
});

var addButton = new UIButton(main, {
	left : 140, 
	top : 185,
	lineHeight : 9,
	paddingLeft : 80,
	height : 40,
	fontSize : 9,
	label : "Add"
});

var h = new UIToolTip(addButton, {
	label : "Click here to see the magic"
});

h.enable();
