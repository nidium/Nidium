/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var body = new Application();
body.className = "body";

var div = new UIView();
div.left = 250;
div.top = 100;
div.width = 300;
div.height = 250;
div.background = "black";
div.textContent = "the brown fox sugar";
body.addChild(div);


var t1 = new TextNode("The quick brown ");
var t2 = new TextNode("fox");
var t3 = new TextNode(" jumps over the lazy dog.");
var t4 = new TextNode("The earliest known appearance of the phrase is from The Michigan School Moderator, a journal that provided teachers with education-related news and suggestions for lessons. In an article titled \"Interesting Notes\" in the March 14, 1885 issue, the phrase is given as a suggestion for writing practice: \"The following sentence makes a good copy for practice, as it contains every letter of the alphabet: A quick brown fox jumps over the lazy dog.\" Note that the phrase in this case begins with the word \"A\" rather than \"The\". Several other early sources also use this variation.");

div.addChild(t1);
div.addChild(t2);
div.addChild(t3);

body.add("UIButton").click(function(){
	echo(div.rooted);
	body.addChild(div);
	echo(div.rooted);
});

window.onready = function(){
	echo("OK, context is ready");
};


/* PRE-REQUIS TO DO ------------------------

1) move all Native events into window

2) add these events : 
	- window.onDOMReady
	- window.onload

3) add a method to createContext();

4) add a method exec("applications/demos/test.js");

*/