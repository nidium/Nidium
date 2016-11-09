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
                if (frame.older == null) {
                    var info = parseFrame(frame);
                    remote.reportError({text: value.unsafeDereference(), frame: info});
                }
            }
        }, this);
    }

    monkeyPatch() {
        console.log = this.log.bind(this);
    }

    reset() {
        this.objectList = new WeakMap();
        this.objectById = new Map();

        this.currentObjectId = 1;        
    }

    run(port) {
        this.wsServer = new WebSocketServer("ws://127.0.0.1:" + port);

        this.wsServer.onopen = (client) => {
            this.reset();
            this.client = client;
            this.onClient();
        }

        this.wsServer.onmessage = (client, message) => {
            var obj = JSON.parse(message.data);
            var call = this.methods.get(obj.method);

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
        this.methods.set(method, callback);
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




var remote = new RemoteDebug();
remote.run(9223);


remote.handle('Runtime.enable', function(reply, params) {
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
}.bind(remote));

remote.handle('Page.canScreencast', function(reply, params) {
    reply({"result":false});
});

remote.handle('Network.canEmulateNetworkConditions', function(reply, params) {
    reply({"result":false});
});

remote.handle('Emulation.canEmulate', function(reply, params) {
    reply({"result":false});
});

remote.handle('Page.getResourceTree', function(reply, params) {
    reply({"frameTree":{"frame":{"id":"22514.2","loaderId":"22514.5","url":"file://","mimeType":"text/html","securityOrigin":"file://"},"resources":[]}});
});

remote.handle('Runtime.getProperties', function(reply, params) {
    if (params.ownProperties || params.accessorPropertiesOnly) {
        let getObjInfo = JSON.parse(params.objectId);

        let objectDescription = this.getObjectDescription(this.getObjectById(getObjInfo.id),
                                    params.accessorPropertiesOnly);

        reply({result: objectDescription});
    } else {
        reply({});
    }
}.bind(remote));

remote.handle('Runtime.callFunctionOn', function(reply, params) {
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

}.bind(remote));

remote.handle('Runtime.evaluate', function(reply, params) {
    let code = params.expression;
    var error = false;
    try {
        var ret = eval.call(this, code);
    } catch(e) {
        error = true;
    }

    reply({result: remote.getValueDescription(ret), wasThrown: error});

}.bind(global));


remote.handle('DOM.getDocument', function(reply, params) {

    if (!document || !document.canvas) {
        reply({});
        return;
    }

    function getTree(root, genesis = false) {
        let children = root.getChildren();
        let tree = {
            nodeId: parseInt(root.id),
            nodeType: 1,
            nodeValue: "",
            nodeName: "CANVAS",
            localName: "canvas",
            attributes: [],
            childNodeCount: children.length,
            children: []
        }

        if (genesis) {
            tree.nodeType = 9;
            tree.nodeName = "#document";
            tree.localName = "";
            tree.documentURL = "file://";
            tree.baseURL = "file://";
            tree.xmlVersion = "";

        }

        for (let child of children) {
            tree.children.push(getTree(child));
        }

        return tree;
    }

    reply({
        root: getTree(document.canvas, true)
    });

}.bind(remote));

remote.onClient = function() {


}

remote.onReady = function()
{
    console.foo();
}
console.log("Debug loaded");
