var self = {};
function XMLHttpRequest() {};

var console = {
	log: function(str) {
		echo("" + str);
	},
    error: function(str) {
        echo("" + str);
    },
	warn: function(str) {
        echo("" + str);
	}
};

self.console = console;

var mgl;
var document = {
	createElement : function() {
        return {
            getContext: function() {
                var v = new NativeGL();
                mgl = v;
                return mgl;
            }
        }
		return v;
	},
	getElementsByTagName: function() {
		return [];
	}
};

Image.prototype.addEventListener = function(ev, callback) {
    this["on" + ev] = callback;
}

window = {};
window.addEventListener = function() {};
requestAnimationFrame = canvas.requestAnimationFrame;
window.innerWidth = 1024;
window.innerHeight = 768;

function XMLHttpRequest() {};
XMLHttpRequest.prototype = {
	onreadystatechange: null,

	open: function(method, url, async) {
		this.method = method;
		this.url = url;
	},

	send: function(args) {
        var h = new Http(this.url + (args ? args : ""));
        h.request(this._onRead.bind(this));
	},

	_onRead: function(response) {
		this.readyState = 4;
		this.status = 200;
		this.responseText = response.data;
		this.response = response.data;

		if (this.onreadystatechange) {
			if (this.responseType == 'arraybuffer') {
                this.response = str2ab(response.data);
			}
			this.onreadystatechange();
		} else {
		log("no onreadystatechange");
		}
	},

	overrideMimeType: function() {},
	setRequestHeader: function() {}
}

function str2ab(str) {
    var buf = new ArrayBuffer(str.length*2); // 2 bytes for each char
    var bufView = new Uint8Array(buf);
    for (var i=0, strLen=str.length; i<strLen; i++) {
        bufView[i] = str.charCodeAt(i);
    }
    return buf;
}

var methods = [
    "activeTexture","attachShader","bindAttribLocation","bindBuffer","bindFramebuffer",
    "bindRenderbuffer","bindTexture","blendColor","blendEquation","blendEquationSeparate","blendFunc",
    "blendFuncSeparate","bufferData","bufferSubData","checkFramebufferStatus","clear","clearColor",
    "clearDepth","clearStencil","colorMask","compileShader","copyTexImage2D","copyTexSubImage2D",
    "createBuffer","createFramebuffer","createProgram","createRenderbuffer","createShader",
    "createTexture","cullFace","deleteBuffer","deleteFramebuffer","deleteProgram","deleteRenderbuffer",
    "deleteShader","deleteTexture","depthFunc","depthMask","depthRange","detachShader","disable",
    "disableVertexAttribArray","drawArrays","drawElements","enable","enableVertexAttribArray","finish",
    "flush","framebufferRenderbuffer","framebufferTexture2D","frontFace","generateMipmap",
    "getActiveAttrib","getActiveUniform","getAttachedShaders","getAttribLocation","getBufferParameter",
    "getContextAttributes","getExtension","getFramebufferAttachmentParameter","getParameter",
    "getProgramParameter","getProgramInfoLog","getRenderbufferParameter","getShaderParameter",
    "getShaderInfoLog","getShaderSource","getSupportedExtensions","getTexParameter","getUniform",
    "getUniformLocation","getVertexAttrib","getVertexAttribOffset","hint","isBuffer","isContextLost",
    "isEnabled","isFramebuffer","isProgram","isRenderbuffer","isShader","isTexture","lineWidth",
    "linkProgram","pixelStorei","polygonOffset","readPixels","releaseShaderCompiler","renderbufferStorage",
    "sampleCoverage","scissor","shaderSource","stencilFunc","stencilFuncSeparate","stencilMask",
    "stencilMaskSeparate","stencilOp","stencilOpSeparate","texParameterf","texParameteri","texImage2D",
    "texSubImage2D","uniform1f","uniform1fv","uniform1i","uniform1iv","uniform2f","uniform2fv","uniform2i",
    "uniform2iv","uniform3f","uniform3fv","uniform3i","uniform3iv","uniform4f","uniform4fv","uniform4i",
    "uniform4iv","uniformMatrix2fv","uniformMatrix3fv","uniformMatrix4fv","useProgram","validateProgram",
    "vertexAttrib1f","vertexAttrib1fv","vertexAttrib2f","vertexAttrib2fv","vertexAttrib3f",
    "vertexAttrib3fv","vertexAttrib4f","vertexAttrib4fv","vertexAttribPointer","viewport", "glGetAttribLocation"
];
if (true) {
	methods.forEach(function(k) {
		var old = WebGLRenderingContext[k];
		WebGLRenderingContext[k] = function() {
            var ret;
            if (!old) {
                echo("  [" + k + "] WebGL method NOT IMPLEMENTED");
                ret = null;
            } else {
                ret = old.apply(this, arguments);
                checkGLError(k);
            }
			return ret;
		}
	});
}

function checkGLError(type) {
	error = mgl.getError();

    if (error == 0) {
	    //echo("["+type+"] OK");
        return;
    }

	errorStr = "";
	switch (error) {
		case 1280:
			errorStr = "GL_INVALID_ENUM";
		break;
		case 1281:
			errorStr = "GL_INVALID_VALUE";
		break;
		case 1282:
			errorStr = "GL_INVALID_OPERATION";
		break;
		case 1283:
			errorStr = "GL_STACK_OVERFLOW";
		break;
		case 1284:
			errorStr = "GL_STACK_UNDERFLOW";
		break;
		case 1285:
			errorStr = "GL_OUT_OF_MEMORY";
		break;
		default:
			errorStr = "UNKNOWN_ERROR :o";
		break;
	}
	echo("["+type+"] error : " + errorStr);
}
