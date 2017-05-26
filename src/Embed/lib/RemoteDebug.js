/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */


const cssParse = require("../modules/css-parse.js");
const Elements = require("Elements");

function camelCase(input) {
    return input.toLowerCase().replace(/-(.)/g, function(match, group1) {
        return group1.toUpperCase();
    });
}

function hyphen(input) {
    return input.replace(/([a-zA-Z])(?=[A-Z])/g, '$1-').toLowerCase(); 
}

class RemoteDebug {

    constructor() {
        this.methods = new Map();
        this.client = undefined;

        this.reset();

        var dbgctx = new DebuggerCompartment();

        dbgctx.run(function(dbg, remote) {

            function parseFrame(frame) {
                var frameName = frame.callee && frame.callee.displayName ? frame.callee.displayName : "anonymous";
                return {
                    "callee": frameName,
                    "script": frame.script.url,
                    "line": frame.script.getOffsetLocation(frame.offset).lineNumber
                }
            }

            dbg.onExceptionUnwind = function(frame, value) {
                console.log("Error", value.unsafeDereference(),JSON.stringify( parseFrame(frame)));
                if (frame.older == null) {
                    var info = parseFrame(frame);
                    remote.reportError({text: value.unsafeDereference(), frame: info});
                }
            }
        }, this);
    }

    monkeyPatch() {
        //console.log = this.log.bind(this);
    }

    reset() {
        this.objectList = new WeakMap();
        this.objectById = new Map();

        this.currentObjectId = 1;        
    }

    run(port = 9223, ip = "127.0.0.1") {
        this.wsServer = new WebSocketServer(`ws://${ip}:${port}`);

        this.wsServer.onopen = (client) => {
            this.reset();
            this.client = client;
            this.onClient();
        }

        this.wsServer.onmessage = (client, message) => {
            var obj = JSON.parse(message.data);
            var call = this.methods.get(obj.method);

            console.log("got", obj.method, (call !== undefined));

            obj.params = obj.params || {};

            if (call === undefined) {
                //console.log("No handler for", obj.method);
                this.reply(client, obj.id);
                return;
            }

            call((result) => {
                this.reply(client, obj.id, result);
            }, obj.params);
        }
    }

    getObjectId(obj) {
        var data = this.objectList.get(obj);
        if (!data) {
            this.objectList.set(obj, {id: this.currentObjectId});
            this.objectById.set(this.currentObjectId, obj);

            return this.currentObjectId++;
        } else {
            return data.id;
        }
    }

    getObjectById(id) {
        return this.objectById.get(id);
    }

    handle(method, callback) {
        if (!Array.isArray(method)) {
            method = [method];
        }

        for (let m of method) {
            this.methods.set(m, callback);
        }
    }

    onClient() {}

    reply(client, id, result = {}) {
        client.send(JSON.stringify({id: id, result: result}));
    }

    call(method, params) {
        if (this.client === undefined) {
            return;
        }

        this.client.send(JSON.stringify({method: method, params: params}));
    }

    getObjectInfoString(obj) {
        return JSON.stringify({injectedScriptId: 1, id: this.getObjectId(obj)});
    }

    getObjectDescription(obj, accessorOnly = false) {

        var ret = [];
        var own = Object.getOwnPropertyNames(obj);

        for (let prop of own) {
            let desc = Object.getOwnPropertyDescriptor(obj, prop);

            if (accessorOnly && (!desc.get && !desc.set)) {
                continue;
            }

            let descRet = {
                writable: desc.writable,
                enumerable: desc.enumerable,
                configurable: desc.configurable,
                name: prop,
                isOwn: true
            };

            let hasValue = !desc.get;

            if (hasValue) {
                descRet.value = this.getValueDescription(obj[prop]);
            } else {
                descRet.get = this.getValueDescription(desc.get);
            }

            if (desc.set) {
                descRet.set = this.getValueDescription(desc.set);
            }

            ret.push(descRet);
        }

        return ret;
    }

    getStringValueDescription(value) {
        let description;

        if (value === undefined) {
            description = "undefined";
        } else if (value === null) {
            description = "null";
        } else {
            description = value.toString();
        }

        return description;
    }

    getValueDescription(value) {
        if (typeof value == "object" && value !== null) {
            return {
                type: "object",
                className: "Object",
                description: "Object",
                objectId: this.getObjectInfoString(value)
            };
        } else if (typeof value == "function") {
            return {
                type: "function",
                className: "Function",
                description: value.toString(),
                objectId: this.getObjectInfoString(value)
            };        
        }

        return {
            type: typeof value,
            value: value,
            description: this.getStringValueDescription(value)
        };
    }

    getCSS(node) {
        /*let stylesProps = [
            "width","height","top","left",
            ].sort();*/

        var canvas = document.getCanvasByIdx(node);

        if (!canvas || !canvas.style) {
            return null;
        }

        let stylesProps = [];
        for (let prop in canvas.style) {
            stylesProps.push(prop);
        }

        stylesProps.sort();

        let returnedProps = [];
        let curcol = 0;
        let cumulText = '';

        for (let style of stylesProps) {
            let val = canvas.style[style];
            if (val == undefined) {
                continue;
            }

            style = hyphen(style);

            let text = `${style}: ${val};`;
            cumulText += text + ' ';

            returnedProps.push({
                name: style,
                value: val + '',
                text: text,
                disabled: false,
                implicit: false,
                range: {
                    startLine: 0,
                    startColumn: curcol,
                    endLine: 0,
                    endColumn: curcol + text.length
                }
            });

            curcol += text.length + 1;
        }

        cumulText = cumulText.trim();

        return {
                /*
                    set the styleSheetId to the canvas idx.
                    When editing the css, the debugger
                    is sending us the styleSheedId rather than the nodeID
                */
                styleSheetId: node + "",
                cssProperties: returnedProps,
                shorthandEntries: [],
                cssText: cumulText,
                range: {
                    startLine: 0,
                    startColumn: 0,
                    endLine: 0,
                    endColumn: cumulText.length
                }            
            }
    }

    onReady(){};

    log(...args) {
        let text = args.join(' ');

        let params = args.map(arg => this.getValueDescription(arg))

        this.call("Console.messageAdded", {
            message: {
                source: "console-api",
                level: "log",
                text: text,
                timestamp: +new Date(),
                type: "log",
                line: 1,
                column: 1,
                url: "file://",
                executionContextId: 1,
                parameters: params
            }
        });
    }

    reportError(error) {


        this.call("Console.messageAdded", {
            message: {
                source: "javascript",
                level: "error",
                text: error.text.toString(),
                timestamp: +new Date(),
                type: "log",
                line: error.frame.line,
                column: 1,
                url: error.frame.script,
                executionContextId: 1,
            }
        });
    }
}


var _remotedebug = new RemoteDebug();

_remotedebug.handle('Runtime.enable', function(reply, params) {
    this.call("Runtime.executionContextCreated", {
        context: {
            id: 1,
            name: "",
            origin: "",
            frameId: "32617.2"
        }
    });

    reply({});

    this.monkeyPatch();

    console.log("       _     _ _                 \n _ __ (_) __| (_)_   _ _ __ ___  \n| '_ \\| |/ _` | | | | | '_ ` _ \\ \n| | | | | (_| | | |_| | | | | | |\n|_| |_|_|\\__,_|_|\\__,_|_| |_| |_|\n\n   Remote debugging activated\n\n");

    this.onReady();
}.bind(_remotedebug));

_remotedebug.handle('Page.canScreencast', function(reply, params) {
    reply({"result":false});
});

_remotedebug.handle('Network.canEmulateNetworkConditions', function(reply, params) {
    reply({"result":false});
});

_remotedebug.handle('Emulation.canEmulate', function(reply, params) {
    reply({"result":false});
});

_remotedebug.handle('Page.getResourceTree', function(reply, params) {
    reply({"frameTree":{"frame":{"id":"22514.2","loaderId":"22514.5","url":"file://","mimeType":"text/html","securityOrigin":"file://"},"resources":[]}});
});

_remotedebug.handle('Runtime.getProperties', function(reply, params) {
    if (params.ownProperties || params.accessorPropertiesOnly) {
        let getObjInfo = JSON.parse(params.objectId);

        let objectDescription = this.getObjectDescription(this.getObjectById(getObjInfo.id),
                                    params.accessorPropertiesOnly);

        reply({result: objectDescription});
    } else {
        reply({});
    }
}.bind(_remotedebug));


/*
    The debugger asks to run some Javascript function with some specific parameters
    This is used for the autocompletion
*/
_remotedebug.handle('Runtime.callFunctionOn', function(reply, params) {
    let getObjInfo = JSON.parse(params.objectId);
    let target = this.getObjectById(getObjInfo.id);

    let func = eval('(' + params.functionDeclaration + ')');

    let args = [];
    let wasThrown = false;

    for (let arg of params.arguments) {
        if (arg.value) {
            args.push(arg.value);
        } else if (arg.type) {
            args.push(arg.type);
        } else if (arg.objectId) {
            args.push(this.getObjectById(arg.objectId));
        } else {
            args.push(undefined);
        }
    }

    try {
        var ret = func.apply(target, args);
    } catch(e) {
        wasThrown = true;
        var ret = e.toString();
    }

    reply({
        result: {
            type: "object",
            wasThrown: wasThrown,
            value: ret
        }
    });

}.bind(_remotedebug));


/*
    The debugger asks to run some Javascript
    This is used for the REPL
*/
_remotedebug.handle('Runtime.evaluate', function(reply, params) {
    let code = params.expression;
    var error = false;
    try {
        var ret = eval.call(this, code);
    } catch(e) {
        error = true;
    }

    reply({result: _remotedebug.getValueDescription(ret), wasThrown: error});

}.bind(global));


/*
    Handle highlighting.
    This is called when the user hover on an element in the DOM view.
*/
_remotedebug.handle('DOM.hideHighlight', function(reply, params) {

    Elements.Node.hideHighlight();
    
    return reply({});
});

_remotedebug.handle('DOM.highlightNode', function(reply, params) {
    var canvas = document.getCanvasByIdx(params.nodeId);
    if (!canvas) {
        console.log("Node not found");
        return reply({});
    }
    try {
        canvas.highlight();
    } catch(e){};
    
    return reply({});
});


_remotedebug.handle('DOM.setNodeValue', function(reply, params) {
    var canvas = document.getCanvasByIdx(params.nodeId);
    if (!canvas) {
        console.log("Node not found");
        return reply({});
    }

    canvas.nodeValue = params.value;

    return reply({});
});

_remotedebug.handle('CSS.getComputedStyleForNode', function(reply, params) {
    console.log("Get conputedstyle")
    var canvas = document.getCanvasByIdx(params.nodeId);

    if (!canvas) {
        console.log("Not found");
        return reply({});
    }

    var ret = [];
    [
        "width",
        "height",
        "marginTop",
        "marginRight",
        "marginBottom",
        "marginLeft",
        "paddingTop",
        "paddingRight",
        "paddingBottom",
        "paddingLeft"
    ].forEach(function(prop) {
        ret.push({
            name: hyphen(prop),
            value: "10px"
        })
    });

    console.log("send computed style");

    return reply({
        computedStyle: ret
    });
});

/*
    Send "styles" for a specific elements
    (usually called when a user click an element).

    We're currently only sending "inline css"
*/
_remotedebug.handle('CSS.getMatchedStylesForNode', function(reply, params) {

    let styles = _remotedebug.getCSS(params.nodeId)
    if (!styles) {
        return reply({});
    }

    reply({
        inlineStyle: styles
    });

});

_remotedebug.handle('CSS.getInlineStylesForNode', function(reply, params) {

    let styles = _remotedebug.getCSS(params.nodeId)
    if (!styles) {
        return reply({});
    }

    reply({
        inlineStyle: styles
    });

});

/*
    The debugger asks for the DOM tree.
    This is used to construct the "Elements" tab
*/
_remotedebug.handle('DOM.getDocument', function(reply, params) {

    if (!document || !document.canvas) {
        return reply({});
    }

    function getTree(root, genesis = false) {
        let children = root.getChildren();

        let nodeName = "canvas";
        if ("name" in root) {
            nodeName = root.tagName || "unknown";
        }

        let tree = {
            nodeId: parseInt(root.idx),
            nodeType: root.nodeType || 1,
            nodeValue: "",
            nodeName: nodeName.toUpperCase(),
            localName: nodeName,
            attributes: [],
            childNodeCount: children.length,
            children: [],
            nodeValue: root.nodeValue
        }

        switch(tree.nodeType) {
            case 3: /* text node */
                root.on("nodeValueChanged", (value) => {
                    _remotedebug.call("DOM.characterDataModified", {
                        nodeId: tree.nodeId,
                        characterData: value
                    });
                });
                break;
            case 1:
                break;
        }

        if (genesis) {
            tree.nodeType = 9;
            tree.nodeName = "document";
            tree.localName = "document";
            tree.documentURL = "file://";
            tree.baseURL = "file://";
            tree.xmlVersion = "";
        } else if (root.attributes) {
            for (let attr in root.attributes) {
                tree.attributes.push(attr, root.attributes[attr] + '')
            }
        }

        //console.log(tree.attributes);

        for (let child of children) {
            tree.children.push(getTree(child));
        }

        return tree;
    }

    reply({
        root: {"nodeId":1,"nodeType":9,"nodeName":"#document","localName":"","nodeValue":"","childNodeCount":2,"children":[
                {"nodeId":2,"nodeType":10,"nodeName":"nml","localName":"","nodeValue":"","publicId":"","systemId":""},
                {"nodeId":3,"nodeType":1,"nodeName":"APPLICATION","localName":"application","nodeValue":"","childNodeCount":1,"children":[getTree(document.canvas, true)],"attributes":[]}
            ],"documentURL":"","baseURL":"","xmlVersion":""}
    });

});

_remotedebug.handle('CSS.setStyleTexts', function(reply, params) {

    if (params.edits.length > 1) {
        console.log("[CSS.setStyleTexts] (warning) received multi edit")
    }

    /*
        We're making styleSheetId (see CSS.getMatchedStylesForNode) 
        matches the canvas idx.
    */
    var canvas = document.getCanvasByIdx(parseInt(params.edits[0].styleSheetId));
    if (!canvas) {
        console.log("Node not found");
        return reply({});
    }


    var css = params.edits[0].text;

    try {
        var ret = cssParse(`element.style { ${css} }`);
    } catch(e) {
        console.log("[CSS.setStyleTexts] (error) Failed to parse CSS");
        return reply({});
    }

    for (let declaration of ret.stylesheet.rules[0].declarations) {
        canvas.style[camelCase(declaration.property)] = declaration.value;
    }

    return reply({
        styles: _remotedebug.getCSS(parseInt(params.edits[0].styleSheetId))
    });
});

_remotedebug.onClient = function() {


}

_remotedebug.onReady = function()
{
    //console.foo();
}

module.exports = _remotedebug;
