/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

{
const Elements = require("Elements");

let reportUnsupported = function(message) {
    console.info("[HTML5Compat] " + message);
}

// {{{ Navigator
var Navigator = (function() {
    function PublicNavigator() {
        throw new Error("Illegal constructor");
    }

    Object.defineProperties(PublicNavigator.prototype, {
        "appCodeName": {
            value: "Nidium",
            writable: false
        },
        "appName": {
            value: "Nidium",
            writable: false
        },
        "appVersion": {
            get: function() {
                return __nidium__.version
            },
        },
        "language": {
            get: function() {
                return (require("OS").language);
            },
        },
        "platform": {
            get: function() {
                return (require("OS").platform);
            },
        },
        "product": {
            value: "Nidium",
            writable: false
        },
        "userAgent": {
            get: function() {
                return this.appName + " / " + __nidium__.version +
                   " (" + this.language + "; rv:" + __nidium__.build + ")";
            }
        },
    });

    PublicNavigator.prototype.vibrate = function() {
        return true;
    };

    var PrivateNavigator = function() {}
    PrivateNavigator.prototype = PublicNavigator.prototype;

    window.navigator = new PrivateNavigator();

    return PublicNavigator;
})();
// }}}

// {{{ HTMLElement (Extending Canvas prototype)
Canvas.prototype.appendChild = function(child) {
    if (child instanceof Canvas) {
        this.add(child);
    } else {
        reportUnsupported("appendChild only supports canvas " + typeof child);
    }
};

(function(){
    let styles = new WeakMap();

    let styleHandler = {
        get: function(target, name) {
            switch (name) {
                case "width":
                case "height":
                    return target[name];
                default:
                    reportUnsupported("Unsupported " + name + " css property");
                break;
            }
        },

        set: function(target, name, value) {
            switch (name) {
                case "width":
                case "height":
                    target[name] = parseInt(value);
                    break;
                default:
                    reportUnsupported("Unsupported assignation of " + name + " css property");
                break;
            }
        }
    }

    for (klass of [Canvas, Image]) {
        Object.defineProperty(klass.prototype, "style", {
            configurable: false,
            get: function(target, name) {
                let style = styles.get(this);
                if (!style) {
                    style = new Proxy(this, styleHandler);
                    styles.set(this, style);
                }

                return style;
            }
        });
    }
})();
//}}}

// {{{ Document
document.createElement = function(what) {
    switch (what) {
        case "canvas":
            return new Canvas(window.innerWidth, window.innerHeight);

        case "img":
            return new Image();

        default:
            return Elements.Create(what);
    }
}

document.createTextNode = function(text) {
    return Elements.Create("textNode", text);
}

document.querySelector = function(sel) {
    return document.getCanvasById(sel);
}

document.createElementNS = function(ns, what) {
    return document.createElement(what);
}

document.addEventListener = function(name, fn) {
    // XXX : Fixme, will not work for all events
    document.canvas.addEventListener(name, fn);
}

document.body = document.canvas;
// }}}

// {{{ XMLHTTPRequest
var XMLHttpRequest = function() {
    this.mimeType = "text/plain";
    this.ev = {};
    this.status = -1;
    this.url = null;
    this.responseType = "text";
}

XMLHttpRequest.prototype = {
    overrideMimeType: function(type) {
        this.mimeType = type;
    },
    setResponseType: function(type) {
        this.responseType = type;
    },
    addEventListener: function(name, fn) {
        if (!this.ev[name]) this.ev[name] = [];

        this.ev[name].push(fn);
    },
    _fireEvent: function(name, ev) {
        if (!this.ev[name]) return;

        for (fn of this.ev[name]) {
            fn.call(this, ev);
        }
    },
    send: function() {
        if (!this.url) return;

        if (this.responseType == "text" || this.responseType == "arraybuffer") {
            let opt = {};
            if (this.responseType == "text") opt.encoding = "utf8";
            File.read(this.url, opt, function(err, data) {
                if (err) {
                    this._fireEvent("error", err);
                    return;
                }

                this.status = 200;

                this._fireEvent("load", {target: {response: data}});
            }.bind(this));
        } else if (this.responseType == "blob") {
            // FIXME : This is probably wrong, it's just a workaround
            // to quickly make three.js works inside Nidium.
            // For a more general approach we should wrap an
            // an ArrayBuffer inside a Blob.
            let f = new File(this.url);
            this.status = 200;
            this._fireEvent("load", {target: {response: f}});
        }
    },
    open: function(method, url) {
        this.method = method;
        this.url = url;
    }
}
// }}}

// {{{ Blob
var URL = {
    _map: new Map(),
    createObjectURL: function(blob) {
        URL._map.set(blob, true);
        return blob;
    },
    revokeObjectURL: function(obj) {
        URL._map.delete(obj);
    }
}
// }}}
}
