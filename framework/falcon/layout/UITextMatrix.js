/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

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
		//echo("-- creating "+n.id+".textLines["+n.linenum+"]");
		n.textLines[n.linenum] = [];
		element._currentIndex = 0;
	}

	
	var node = null;
	element._currentIndex = 0;
	element._currentLine = line;

	for (i=0; i<nb_chars; i++){
		var char = textLine[i],
			index = element.textLength,
			letterWidth = getTextPixelWidth(element, char);

		node = element.charElements[index];

		if (textAlign=="justify"){
			if (char == " "){
				letterWidth += spacing;
			} else {
				letterWidth -= spacing*nb_spaces/nb_letters;
			}
		}

		var pos = offLeft + offset + position + offgap;

		if (element._currentNode !== node) {
			//echo("--- new node", node.id);

			element._currentNode = node;
			element._currentIndex = 0;

			node.linenum = 0;
			node.textLines[0] = [];

			//echo("-- node "+node.id+".textLines["+node.linenum+"]");
			node.textLines[node.linenum] = [];

		}

		/*
		echo(
			i+","+index, ":",
			node.id+".textLines["+node.linenum+"]["+element._currentIndex+"] = "+char
		);
		*/

		letters[i] = {
			index : index,
			node : node,
			char : char,
			position : pos,
			width : letterWidth,
			lineHeight : node.lineHeight,
			linegap : linegap,
			selected : false
		};

		node.textLines[node.linenum][element._currentIndex] = {
			char : char,
			line : line,
			position : pos,
			width : letterWidth,
			linegap : linegap
		};

		position += letterWidth;
		offgap += gap;

		element.textLength++;
		element._currentIndex++;
	}

	// last letter Position Approximation Corrector

	if (letters[nb_chars-1]) {
		var last = letters[nb_chars-1],
			delta = fitWidth - (last.position + last.width);

		if ((0.05 + last.position + last.width) > fitWidth+offLeft) {
			last.position = Math.floor(last.position - delta - 0.5) - offLeft;
		}

		var pos = offLeft + offset + position + offgap;

		letters[i] = {
			char : " ",
			position : pos,
			width : spacewidth,
			linegap : linegap,
			selected : false
		};

		//echo(i, index, element._currentIndex);
		node.textLines[node.linenum][element._currentIndex] = {
			char : " ",
			line : line,
			position : pos,
			width : spacewidth,
			linegap : linegap
		};
		element._currentIndex++;

	}

	element._currentNode = node;
	node.linenum++;
	
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
	return element.layer.context.measureText(text)
}

function getLastLineTextAlign(element){
	return element.textAlign == "justify" ? "left" : element.textAlign;
}

function getParagrapheMatrix(element, paragraphe){
	var	k = 0,
		idx = 1,
		line = 0,
		fitWidth = element.maxWidth,
		wordsArray = [],
		textAlign = element.textAlign,
		words = paragraphe.split(' ');

	element._tmpMatrix = [];

	while (words.length>0 && idx <= words.length) {
		var str = words.slice(0, idx).join(' '),
			linePixelWidth = getTextPixelWidth(element, str);

		//echo(linePixelWidth);

		if (linePixelWidth > fitWidth) {
			idx = (idx == 1) ? 2 : idx;

			wordsArray = words.slice(0, idx - 1);
			updateMatrix(element, line++, wordsArray, textAlign, fitWidth);

			k++;
			words = words.splice(idx - 1);
			idx = 1;

		} else {
			idx++;
		}

	}

	// last line
	if (idx > 0) {
		textAlign = getLastLineTextAlign(element);
		updateMatrix(element, line, words, textAlign, fitWidth);
	}

	line++;

	return this._tmpMatrix;
}
