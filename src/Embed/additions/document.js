/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements = require("Elements");

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

    const body = document.createElement("layout");
    document.canvas.add(body);

    Object.defineProperty(document, "body", {
        enumerable: false,
        configurable: false,
        writable: false,
        value: body
    });

    Object.defineProperty(document, "documentElement", {
        enumerable: false,
        configurable: false,
        writable: false,
        value: body
    });
}
