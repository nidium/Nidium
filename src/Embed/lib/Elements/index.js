/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = module.exports = {};

const ShadowRoot    = require("../../ShadowRoot.js");
const g_MainShadow  = new ShadowRoot(document.canvas, {"name": "main"});

Object.defineProperty(document.canvas, "shadowRoot", {
    "writable": false,
    "configurable": false,
    "value": g_MainShadow
});

Elements.currentShadow = null;

Elements.NodeType = {
    ELEMENT_NODE : 1,
    TEXT_NODE : 3,
    PROCESSING_INSTRUCTION_NODE : 7,
    COMMENT_NODE : 8,
    DOCUMENT_NODE : 9,
    DOCUMENT_TYPE_NODE : 10,
    DOCUMENT_FRAGMENT_NODE : 11
};

Elements.Create = function(tag, attributes, shadowRoot=g_MainShadow) {
    tag = tag.toLowerCase();
    let ret;

    let previousShadow = Elements.currentShadow;
    Elements.currentShadow = shadowRoot;

    try {
        if (!(tag in Elements)) {
            throw Error(`<${tag}> is not implemented.`);
            return;
        }

        if (!Elements[tag].__NodeName__) Elements[tag].__NodeName__ = tag;

        ret = new Elements[tag](attributes);
    } finally {
        Elements.currentShadow = previousShadow;
    }

    return ret;
}

Elements.Exists = function(tag) {
    return (tag.toLowerCase() in Elements);
}

/*
    Generic resource loader : return the text content
    of the file pointed by src attribute
*/
Elements.Loader = function(attributes) {
    let data;

    if (attributes && attributes.src) {
        let src = attributes.src;
        try {
            data = File.readSync(src, {"encoding": "utf-8"});
        } catch (e) {
            console.error(`Failed to load ${src}: ${e}`);
            return "";
        }

        if (!data) {
            console.warn(`${src} is empty.`);
            return "";
        }
    }

    return data;
}

/*
 * Native UI Controls and NML tags
 */
load("embed://lib/Elements/controls/node.js");
load("embed://lib/Elements/controls/element.js");
load("embed://lib/Elements/controls/layout.js");
//load("embed://lib/Elements/controls/button.js");
load("embed://lib/Elements/controls/canvas.js");
load("embed://lib/Elements/controls/div.js");
load("embed://lib/Elements/controls/span.js");
load("embed://lib/Elements/controls/img.js");
load("embed://lib/Elements/controls/section.js");
load("embed://lib/Elements/controls/textnode.js");

/**
 * Node extends
 */
load("embed://lib/Elements/nss.js");
load("embed://lib/Elements/script.js");
load("embed://lib/Elements/template.js");
load("embed://lib/Elements/component.js");
load("embed://lib/Elements/slot.js");

window._onready = function(lst){};

module.exports = Elements;
