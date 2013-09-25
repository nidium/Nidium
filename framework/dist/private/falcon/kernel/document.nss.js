/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

document.nss = {
	getSelector : function(selector){
		return document.stylesheet[selector];
	},

	setSelector : function(selector, properties){
		this.setProperties(selector, properties);
	},

	addSelector : function(selector, properties){
		this.mergeProperties(selector, properties);
	},

	/* -------------------------------------------------------- */

	/* add to the existing stylesheet document */
	add : function(sheet){
		for (var selector in sheet){
			if (sheet.hasOwnProperty(selector)){
				this.mergeProperties(selector, sheet[selector]);
			}
		}
	},

	/* replace the existing stylesheet document with a new one */
	set : function(sheet){
		document.stylesheet = {};
		this.add(sheet);
	},

	refresh : function(){
		this.add(document.stylesheet);
	},

	/* load a local or distant stylesheet */
	load : function(url, sync=true){
		var ____self____ = this,
			sheet = {};

		if (sync) {
			/* the synchronous way uses built-in method load */
			sheet = load(url, 'nss');
			this.add(sheet);
		} else {
			/* the asynchronous way */
			File.getText(url, function(content){
				var sheetText = ____self____.parse(content);
				try {
					eval("____self____.add(" + sheetText + ")");
				} catch (e) {
					throw ('Error parsing Native StyleSheet "'+url+'"');
				}
			});
		}
	},

	/* apply style to elements that match selector */
	updateElements : function(selector){
		document.getElementsBySelector(selector).each(function(){
			this.applyStyleSheet();
		});
	},

	/* merge properties in an existing selector, or create new one */
	mergeProperties : function(selector, properties){
		var prop = document.stylesheet[selector];
		if (prop) {
			for (var p in properties){
				if (properties.hasOwnProperty(p)){
					prop[p] = properties[p];
				}
			}
		} else {
			document.stylesheet[selector] = properties;
		}
		this.updateElements(selector);
	},

	setProperties : function(selector, properties){
		document.stylesheet[selector] = properties;
		this.updateElements(selector);
	},

	getProperties : function(selector){
		return document.stylesheet[selector];
	},

	/*
	 * NSS Parser, adapted from James Padolsey's work
	 */
	parse : function(text){
		text = ('__' + text + '__').split('');

		var mode = {
			singleQuote : false,
			doubleQuote : false,
			regex : false,
			blockComment : false,
			lineComment : false
		};

		for (var i = 0, l = text.length; i < l; i++){
			if (mode.regex){
				if (text[i] === '/' && text[i-1] !== '\\'){
					mode.regex = false;
				}
				continue;
			}

			if (mode.singleQuote){
				if (text[i] === "'" && text[i-1] !== '\\'){
					mode.singleQuote = false;
				}
				continue;
			}

			if (mode.doubleQuote){
				if (text[i] === '"' && text[i-1] !== '\\'){
					mode.doubleQuote = false;
				}
				continue;
			}

			if (mode.blockComment){
				if (text[i] === '*' && text[i+1] === '/'){
				text[i+1] = '';
					mode.blockComment = false;
				}
				text[i] = '';
				continue;
			}

			if (mode.lineComment){
				if (text[i+1] === '\n' || text[i+1] === '\r'){
					mode.lineComment = false;
				}
				text[i] = '';
				continue;
			}

			mode.doubleQuote = text[i] === '"';
			mode.singleQuote = text[i] === "'";

			if (text[i] === '/'){
				if (text[i+1] === '*'){
					text[i] = '';
					mode.blockComment = true;
					continue;
				}
				if (text[i+1] === '/'){
					text[i] = '';
					mode.lineComment = true;
					continue;
				}
				mode.regex = true;
			}
		}

		return text.join('')
				.slice(2, -2)
				.replace(/[\n\r]/g, '')
				.replace(/\s+/g, ' ');
	}
};

/* -------------------------------------------------------------------------- */
