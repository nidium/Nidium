// JS1K ヽ(*⌒∇⌒*)ﾉ

var text = [];

text.push("public : {");
text.push("    backgroundImage : {");
text.push("        value : function(){");
text.push("            return OptionalString(this.options.backgroundImage, null);");
text.push("        },");
text.push("");
text.push("        set : function(value){");
text.push("            if (value) {");
text.push("                this.setBackgroundURL(value);");
text.push("            }");
text.push("        }");
text.push("    },");
text.push("");
text.push("    scrollLeft : {");
text.push("        set : function(value){");
text.push("            var max = this.scrollable ?");
text.push("                            this.layer.contentWidth - this._width : 0;");
text.push("");
text.push("            this.scrollLeft = Math.min(value, max);");
text.push("            this.refreshScrollBars();");
text.push("        }");
text.push("    },");
text.push("");
text.push("    scrollTop : {");
text.push("        set : function(value){");
text.push("            var max = this.scrollable ?");
text.push("                            this.layer.contentHeight - this._height : 0;");
text.push("");
text.push("            this.scrollTop = Math.min(value, max);");
text.push("            this.refreshScrollBars();");
text.push("        }");
text.push("    },");
text.push("");
text.push("    scrollBottom : {");
text.push("        set : function(value){");
text.push("            var max = this.scrollable ?");
text.push("                            this.layer.contentWidth - this._width : 0;");
text.push("");
text.push("            this.scrollLeft = Math.min(value, max);");
text.push("            this.refreshScrollBars();");
text.push("        }");
text.push("    },");
text.push("    ");
text.push("    scrollRight : {");
text.push("        set : function(value){");
text.push("            var max = this.scrollable ?");
text.push("                            this.layer.contentHeight - this._height : 0;");
text.push("");
text.push("            this.scrollTop = Math.min(value, max);");
text.push("            this.refreshScrollBars();");
text.push("        }");
text.push("    }");
text.push("}");
text.push("");
text.push("");
text.push("Ready.");

var so = 0, 
	line = 1,
	fontSize = 18,
	lineHeight = fontSize+4;

var W = 1024,
	H = text.length*lineHeight;

var canvas = Native.canvas,
	ctx = canvas.getContext("2d");

canvas.height = H;
ctx.textBaseline = "top";

ctx.fontSize = fontSize;
ctx.fontType = "console";

var timer = setInterval(function() {
	//ctx.globalCompositeOperation = "source-over";
	ctx.shadowBlur = 0;

	ctx.fillStyle = "rgba(0, 25, 0, 0.24)";
	ctx.fillRect(0, 0, W, H);

	ctx.shadowColor = "rgba(0, 255, 0, 0.20)";
	ctx.shadowBlur = 6;

	ctx.fillStyle = "#003300";
	//ctx.globalCompositeOperation = "lighter";

	text.forEach(function(v, i){
		if (i<=line) {
			if (i==line) {
				v = v.substr(0, so);
			}
			ctx.fillText(v, 20, 20+i*lineHeight);
		}
	});

	ctx.fillStyle = "#00ff00";
	ctx.fillRect(
		20+ctx.measureText(text[line].substr(0, so)),
		24+line*lineHeight,
		fontSize-2,
		fontSize+4
	);

	so++;

	if (so >= text[line].length) {
		line++;
		so = 0;
	}

	if (line*lineHeight > H-550) {
		canvas.top -= 1;
	}
	
	if (line >= text.length) {
		clearInterval(timer);
	}
}, 16);

var c = new Canvas(100, 100);


url = "../demos/oldscreen/oldscreen.s";

Uint8Array.prototype.toString = function(){
	return String.fromCharCode.apply(null, new Uint8Array(this));
};

ArrayBuffer.prototype.toString = function(){
	return String.fromCharCode.apply(null, new Uint8Array(this));
};

File.getText = function(url, callback){
	var f = new File(url);
	f.open("r", function(){
		f.read(f.filesize, function(buffer){
			this.size = buffer.byteLength;
			this.buffer = buffer;
			if (typeof callback == "function") callback.call(this, this.buffer.toString());
		});
	});
};

File.getText(url, function(source){
	var program = ctx.attachGLSLFragment(source);
});

/*

var source = [
	"uniform sampler2D Texture;",

	"void main(void) {",
		"gl_FragColor = texture2D(Texture, gl_TexCoord[0].xy);",
	"}"
].join('\n');

var program = ctx.attachGLSLFragment(source);

*/