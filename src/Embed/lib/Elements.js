/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = module.exports = {};

const ShadowRoot    = require("../ShadowRoot.js");

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

Elements.Create = function(tag, attributes, shadowRoot=document.canvas.shadowRoot) {
    tag = tag.toLowerCase();
    let ret;

    let previousShadow = Elements.currentShadow;
    Elements.currentShadow = shadowRoot;

    try {
        if (!(tag in Elements)) {
            throw Error(`<${tag}> is not implemented.`);
            return;
        }

        if (!Elements[tag].tagName) {
            Object.defineProperty(Elements[tag], '_tagName', {
                enumerable: false,
                configurable: false,
                writable: false,
                value: tag
            });
        }

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

module.exports = Elements;

