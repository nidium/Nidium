/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

DOMElement.nodes = {
	updateCharNodes : function(node, parent){
		var text = node.text;

		node.linenum = 0;
		node.textLines = [];

		for (var i=0; i<text.length; i++){
			parent.charElements.push(node);
		}
	},

	updateTextNodes : function(children){
		for (var i=0; i<children.length; i++){
			var node = children[i];

			if (node.flags & FLAG_TEXT_NODE){
				node.updateElement();
				node.redraw();
			}
		}
	},

	setWholeTextRenderingMatrix : function(element){
		var	paragraphes = element.wholeText.split(/\r\n|\r|\n/);

		element.matrix = [];
		element.textLength = 0;

		setTextContext(element);

		for (var i = 0; i < paragraphes.length; i++) {
			var matrix = getParagrapheMatrix(element, paragraphes[i]);
			element.matrix.push(matrix);
		}
	},

	resetTextNodes : function(p){
		var children = p.childNodes,
			txt = [];

		p.wholeText = "";
		p.charElements = [];
		p.maxWidth = p.maxWidth ? p.maxWidth : p.contentWidth;
		p._currentNode = null;

		for (var i=0; i<children.length; i++){
			var node = children[i];

			if (node.flags & FLAG_TEXT_NODE){
				this.updateCharNodes(node, p);
				txt.push(node.text);
			}
		}

		p.wholeText = txt.join('');

		this.setWholeTextRenderingMatrix(p);
		this.updateTextNodes(children);
	}
};
