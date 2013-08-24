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
			parent.nodeAtIndex.push(node);
		}
	},

	updateTextNodes : function(children){
		for (var i = children.length-1; i>0; --i){
			var node = children[i];

			if (node.flags & FLAG_TEXT_NODE){
				node.updateElement();
				node.redraw();
			}
		}
	},

	setWholeTextRenderingMatrix : function(element){
		var	paragraphes = element.wholeText.split(/\r\n|\r|\n/);

		console.log("-------------------------------------------------------------");

		element.matrix = [];
		element.textLength = 0;
		element.linenum = 0;

		for (var i=0; i<paragraphes.length; i++) {
			//console.log("********** paragraphe", i);
			var matrix = getParagrapheMatrix(i, element, paragraphes[i]);
			element.matrix.push(matrix);
		}

	},

	refresh : function(p){
		var children = p.childNodes;
		
		p._currentNode = null;
		p.maxWidth = p.maxWidth ? p.maxWidth : p.contentWidth;

		for (var i=0; i<children.length; i++){
			var node = children[i];

			if (node.flags & FLAG_TEXT_NODE){
				node.linenum = 0;
				node.textLines = [];
			}
		}

		this.setWholeTextRenderingMatrix(p);
		this.updateTextNodes(children);
	},

	resetTextNodes : function(p){
		var children = p.childNodes,
			txt = [];

		p.wholeText = "";
		p.nodeAtIndex = [];
		p.maxWidth = p.maxWidth ? p.maxWidth : p.contentWidth;

		for (var i=0; i<children.length; i++){
			var node = children[i];

			if (node.flags & FLAG_TEXT_NODE){
				this.updateCharNodes(node, p);
				txt.push(node.text);
			}
		}

		p.wholeText = txt.join('');
		this.refresh(p);
	}
};
