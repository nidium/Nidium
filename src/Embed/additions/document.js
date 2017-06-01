/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements      = require("Elements");
    const ShadowRoot    = require("../ShadowRoot.js");

    /*
        Create document ShadowRoot & document.canvas
    */
    load("embed://framework/elements/node.js");
    load("embed://framework/elements/element.js");
    load("embed://framework/elements/layout.js");

    const mainShadowRoot = new ShadowRoot(null, {"name": "main"});

    Elements.currentShadow = mainShadowRoot;
    const mainCanvas = Elements.Create("layout", {}, mainShadowRoot);
    Elements.currentShadow = null;

    Object.defineProperty(document, 'canvas', {
        enumerable: false,
        configurable: false,
        writable: false,
        value: mainCanvas
    });

    mainShadowRoot._setHost(document.canvas);

    document.canvas.style.position = "relative";
    document.canvas.style.width = "100%";
    document.canvas.style.height = "100%";

    Object.defineProperty(document, 'body', {
        enumerable: false,
        configurable: false,
        writable: false,
        value: mainCanvas
    });

    document.addToRootCanvas(document.canvas);

    document.getElementById = function(id){
        return document.canvas.shadowRoot.getElementById(id);
    };

    document.createElement = function(name) {
        return Elements.Create(name);
    }

    document.createComment = function(data) {
        return Elements.Create("comment", data);
    }

    document.createTextNode = function(data) {
        return Elements.Create("textnode", data);
    }

    Elements.documentfragment = class extends Elements.layout {}
    document.createDocumentFragment = function() {
        return Elements.Create("DocumentFragment");
    }

    Object.defineProperty(document, "documentElement", {
        enumerable: false,
        configurable: false,
        writable: false,
        value: document.body
    });
}
