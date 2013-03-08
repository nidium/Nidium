/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

function setLetter(letters, i, index, node, char, pos, letterWidth, linegap){
	letters[i] = {
		index : index,
		node : node,
		char : char,
		position : Math.round(pos),
		width : Math.round(letterWidth),
		lineHeight : node.lineHeight,
		linegap : linegap,
		selected : false
	};
}

function mapLetter(node, index, char, line, pos, letterWidth, linegap){
	node.textLines[node.linenum][index] = {
		char : char,
		line : line,
		position : Math.round(pos),
		width : Math.round(letterWidth),
		lineHeight : node.lineHeight,
		linegap : linegap
	};
}

var setNextNode = function(element, node){
	element._currentNode = node;
	element._currentIndex = 0;
	node.linenum = 0;
	node.textLines[0] = [];
	node.textLines[node.linenum] = [];
};

function getLetters(element, line, words, textAlign, offLeft, fitWidth){
	var context = element.layer.context,
		fontSize = element.fontSize,
		fontType = element.fontType,

		textLine = words.join(' '),
		
		nb_words = words.length,
		nb_spaces = nb_words-1,
		nb_chars = textLine.length,
		nb_letters = nb_chars - nb_spaces,

		spacewidth = getTextPixelWidth(element, " "),
		spacing = (textAlign == "justify") ? fontSize/3 : fontSize/10,

		i = 0,
		linegap = 0,
		gap = 0,
		offgap = 0,
		offset = 0,

		position = 0,
		letters = [];

	linegap = fitWidth - getTextPixelWidth(element, textLine);


/* ---------------------------------- */
/* ---------------------------------- */
/* ---------------------------------- */
var debug = false;
/* ---------------------------------- */
/* ---------------------------------- */
/* ---------------------------------- */

	switch(textAlign) {
		case "justify" :
			gap = nb_chars > 1 ? (linegap/(nb_chars-1)) : 0;
			break;
		case "right" :
			offset = linegap;
			break;
		case "center" :
			offset = linegap/2;
			break;
		default :
			break;
	}

	if (element._currentNode) {
		var n = element._currentNode;
		n.textLines[n.linenum] = [];
		element._currentIndex = 0;
	}

	if (debug)
	echo(
		"line", line,
		"nb_words:", nb_words,
		"nb_spaces:", nb_spaces,
		"nb_chars:", nb_chars,
		"nb_letters:", nb_letters
	);
	
	var node = null;
	element._currentIndex = 0;
	element._currentLine = line;


	if (nb_chars == 0) {
		var idx = element.textLength;
		var node = element.nodeAtIndex[idx];
		if (node && element.wholeText.charAt(idx) == "\n") {
			setNextNode(element, node);
			if (debug)
			echo(
				"KKKK", i+","+idx, ":",
				node.id+".textLines["+node.linenum+"]["+element._currentIndex+"] = "+"[LINEBREAK]"
			);

			setLetter(letters, 0, idx, node, "\n", 0, fitWidth, fitWidth);
			mapLetter(node, 0, "\n", line, 0, fitWidth, fitWidth);

		//	echo(">>>>>>>>>>>", node.id, node.textLines[0][0].char);

			element.textLength++;
			node.linenum++;
		}
		return false;
	}


	for (i=0; i<nb_chars; i++){
		var char = textLine[i],
			index = element.textLength,
			letterWidth = Math.round(getTextPixelWidth(element, char));

		node = element.nodeAtIndex[index];

		if (textAlign=="justify"){
			if (char == " "){
				letterWidth += spacing;
			} else {
				//letterWidth -= spacing*nb_spaces/nb_letters;
			}
		} else {
			if (char == "\t"){
				letterWidth = spacing*4*6;
			}
		}

		var pos = offLeft + offset + position + offgap;

		if (element._currentNode !== node) {
			setNextNode(element, node);
		}

		if (debug)
		echo(
			i+","+index, ":",
			node.id+".textLines["+node.linenum+"]["+element._currentIndex+"] = "+char
		);

		var lw = letterWidth;
		setLetter(letters, i, index, node, char, pos, lw, linegap);
		mapLetter(node, element._currentIndex, char, line, pos, lw, linegap);

		position += letterWidth;
		offgap += gap;

		element.textLength++;
		element._currentIndex++;
	}

	// last letter Position Approximation Corrector

	if (letters[nb_chars-1]) {
		var last = letters[nb_chars-1],
			delta = fitWidth - (last.position + last.width);

		var char = element.lineBreakSeparator == " " ? " " : "\n",
			index = element.textLength,
			letterWidth = getTextPixelWidth(element, char);

		if ((0.05 + last.position + last.width) > fitWidth+offLeft) {
			last.position = Math.floor(last.position - delta - 0.5) - offLeft;
		}

		var pos = offLeft + offset + position + offgap;

		if (debug)
		echo(
			i+","+index, ":",
			node.id+".textLines["+node.linenum+"]["+element._currentIndex+"] = "+ (element.lineBreakSeparator == " " ? '[SPACE]' : '[LINEBREAK]')
		);

		var lw = spacewidth;
		setLetter(letters, i, index, node, char, pos, lw, linegap);
		mapLetter(node, element._currentIndex, char, line, pos, lw, linegap);

		element.textLength++;
		element._currentIndex++;
	}

	if (node !== null) {
		element._currentNode = node;
	} else {
		node = element._currentNode;

		if (debug)
		echo(
			'BREAK', i, line,
			node.id+".textLines["+node.linenum+"]["+element._currentIndex+"] = "+ '[LINEBREAK]'
		);
		setLetter(letters, i, index, node, "\n", 0, fitWidth, fitWidth);
		mapLetter(node, 0, "\n", line, 0, fitWidth, fitWidth);

		element.textLength++;
	}

	element._currentNode.linenum++;
	
	return letters;
}

function updateMatrix(element, line, words, textAlign, fitWidth){
	element._tmpMatrix[line] = {
		text : words.join(' '),
		align : textAlign,
		words : words,
		letters : getLetters(element, line, words, textAlign, 0, fitWidth)
	};
}

function setTextContext(element){
	var context = element.layer.context;
	context.fontSize = element.fontSize;
	context.fontType = element.fontType;
}

function getTextPixelWidth(element, text){
	setTextContext(element);
	return element.layer.context.measureText(text);
}

function getLastLineTextAlign(element){
	return element.textAlign == "justify" ? "left" : element.textAlign;
}

function getParagrapheMatrix(p, element, paragraphe){
	var	idx = 1,
		line = 0,
		fitWidth = element.maxWidth,
		wordsArray = [],
		textAlign = element.textAlign,
		words = paragraphe.split(' '),

		lastWordWidth = 0,
		lastLinePixelWidth = 0;

	element._tmpMatrix = [];

//	echo("-------------------------------------------------------------");

	while (words.length>0 && idx <= words.length) {
		var str = words.slice(0, idx).join(' '),
			linePixelWidth = Math.round(getTextPixelWidth(element, str)),
			lastWord = words[idx-1];

		lastWordWidth = Math.round(linePixelWidth - lastLinePixelWidth);

//		echo(element.linenum, idx, linePixelWidth, str, "("+lastWordWidth+")");

		lastLinePixelWidth = Math.round(linePixelWidth);

		if (linePixelWidth > fitWidth) {

			if (lastWordWidth <= fitWidth) {
				idx = (idx == 1) ? 2 : idx;

				wordsArray = words.slice(0, idx-1);
				element.lineBreakSeparator = " ";
	
				updateMatrix(element, element.linenum++, wordsArray, textAlign, fitWidth);

				words = words.splice(idx-1);
				idx = 1;
				lastLinePixelWidth = 0;
			} else {
				
				// TODO : handle large unsplitable words
				//lastWord = lastWord.split('').join(String.fromCharCode('8203'));
				//split(/\r\n|\r|\n/);
				echo(lastWord);
				echo("*** word overflow", lastWordWidth - fitWidth);
			}

		} else {
			idx++;
		}

	}

	// last line
	if (idx > 0) {
		textAlign = getLastLineTextAlign(element);
		element.lineBreakSeparator = "\n";
		updateMatrix(element, element.linenum, words, textAlign, fitWidth);
	}

	element.linenum++;

	return this._tmpMatrix;
}
