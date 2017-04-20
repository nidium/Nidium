/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */



"use strict";
// Creates and returns a Proxy wrapping a target so that all changes can be trapped and forwarded to
// a callback. The callback takes an array of changes just like the traditional original Chrome Object.observe
// {object:<object changed>,name:<field changed>,type:add|update|delete|reconfigure|preventExtensions|setPrototype,oldValue:<old value if update | delete>}
// The acceptlist can be add|update|delete|reconfigure|preventExtensions|setPrototype.
// v 0.0.10 to support pausing and restarting observation two additional constructor arguments are available to Object.observe:
// pausable - create the Observer so it can be paused
// pause - create observer in paused state
// if pausable is true then an additional method deliver(ignorePrevious) is available to start delivery
// to pause delivery set a property called pause on the function deliver to true
// pausable is optional to reduce the chance of shadowing a property or method on any existing code called deliver
    
function Observer(target, callback, acceptlist, pausable, pause, delay) {
    var me = this, proxy;

    function deliver(ignorePrevious, delay) {
        deliver.delay = delay;

        if (!deliver.pause) {
            if (me.changeset.length>0) {
                if (!ignorePrevious) {
                    var changes = me.changeset.filter(function(change){
                        return !acceptlist || acceptlist.indexOf(change.type)>=0;
                    });

                    if (changes.length>0) {
                        callback(changes);
                    }
                }
                me.changeset = [];
            }
        }
    }

    deliver.pause = pause;
    deliver.delay = delay;

    me.get = function(target, property) {
        if (property==="__observer__") {
            return me;
        }

        if (property==="unobserve") {
            return function() {
                Object.unobserve(target);
                return target;
            };
        }

        if (property==="deliver") {
            return deliver;
        }
        return target[property];
    };

    me.target = target;
    me.changeset = [];

    if (!me.target.__observerCallbacks__) {
        /*
            __observerCallbacks__ is used as an index to get at the proxy
            which is the observer, so we can unobserve
        */
        Object.defineProperty(target, "__observerCallbacks__", {
            enumerable: false,
            configurable: true,
            writable: false,
            value: []
        });

        Object.defineProperty(target,"__observers__", {
            enumerable: false,
            configurable: true,
            writable: false,
            value: []
        });
    }

    me.target.__observerCallbacks__.push(callback);
    me.target.__observers__.push(this);

    proxy = new Proxy(target, me);
    deliver(false, delay);
    return proxy;
}

Observer.prototype.deliver = function() {
    return this.get(null, "deliver");
};

Observer.prototype.set = function(target, property, value) { // , receiver
    var oldvalue = target[property];
    var type = (oldvalue===undefined ? "add" : "update");
    target[property] = value;
    
    if (target.__observers__.indexOf(this)>=0 && (!this.acceptlist || this.acceptlist.indexOf(type)>=0)) {
        var change = {object:target, name:property, type:type},
            start = this.changeset.length === 0,
            deliver = this.deliver();

        if (type==="update") {
            change.oldValue = oldvalue;
        }

        this.changeset.push(change);

        if (start) {
            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
        }
    }
    return true;
};

Observer.prototype.deleteProperty = function(target, property) {
    var oldvalue = target[property];

    delete target[property];

    if (target.__observers__.indexOf(this)>=0 && !this.acceptlist || this.acceptlist.indexOf("delete")>=0) {
        var change = {object:target, name:property, type:"delete", oldValue:oldvalue},
            start = this.changeset.length === 0,
            deliver = this.deliver();

        this.changeset.push(change);

        if (start) {
            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
        }
    }

    return true;
};

Observer.prototype.defineProperty = function(target, property, descriptor) {
    Object.defineProperty(target, property, descriptor);
    
    if (target.__observers__.indexOf(this)>=0 && !this.acceptlist || this.acceptlist.indexOf("reconfigure")>=0) {
        var change = {object:target, name:property, type:"reconfigure"},
            start = this.changeset.length === 0,
            deliver = this.deliver();

        this.changeset.push(change);

        if (start) {
            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
        }
    }
    return true;
};

Observer.prototype.setPrototypeOf = function(target, prototype) {
    var oldvalue = Object.getPrototypeOf(target);

    Object.setPrototypeOf(target, prototype);

    if (target.__observers__.indexOf(this)>=0 && !this.acceptlist || this.acceptlist.indexOf("setPrototype")>=0) {
        var change = {object:target, name:"__proto__", type:"setPrototype", oldValue:oldvalue},
            start = this.changeset.length === 0,
            deliver = this.deliver();

        this.changeset.push(change);

        if (start) {
            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
        }
    }
    return true;
};

Observer.prototype.preventExtensions = function(target) {
    Object.preventExtensions(target);

    if (target.__observers__.indexOf(this)>=0 && !this.acceptlist || this.acceptlist.indexOf("preventExtensions")>=0) {
        var change = {object:target, type:"preventExtensions"},
            start = this.changeset.length === 0,
            deliver = this.deliver();

        this.changeset.push(change);

        if (start) {
            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
        }
    }
    return true;
};

Object.observe = function(object, callback, acceptlist, pausable, pause, delay) {
    return new Observer(object, callback, acceptlist, pausable, pause, delay);
};

Object.unobserve = function(object, callback) {
    if (object.__observerCallbacks__) {
        if (!callback) {
            object.__observerCallbacks__.splice(0, object.__observerCallbacks__.length);
            object.__observers__.splice(0, object.__observers__.length);
            return;
        }

        object.__observerCallbacks__.forEach(function(observercallback, i){
            if (callback===observercallback) {
                object.__observerCallbacks__.splice(i, 1);
                delete object.__observers__[i].callback;
                object.__observers__.splice(i, 1);
            }
        });
    }
};

Array.observe = function(object, callback, acceptlist, pausable, pause, delay) {
    if (!(object instanceof Array) && !Array.isArray(object)) {
        throw new TypeError("First argument to Array.observer is not an Array");
    }

    var acceptlist = acceptlist || ["add", "update", "delete", "splice"];

    var arrayproxy = new Proxy(object, {
        get: function(target, property) {
            if (property==="unobserve") {
                return function(callback) {
                    if (callback) {
                        return Object.unobserve(target,callback);
                    }
                    return target.unobserve();
                };
            }

            if (property==="splice") {
                return function(start, end) {
                    if (typeof(start)!=="number" || typeof(end)!=="number") {
                        throw new TypeError("First two arguments to Array splice are not number, number");
                    }
                    var removed = this.slice(start, start+end),
                        addedCount = (arguments.length > 1 ? arguments.length-2 : 0),
                        change =  {object:object, type:"splice", index:start, removed:removed, addedCount:addedCount};

                    target.splice.apply(target,arguments);

                    if (acceptlist.indexOf("splice")>=0) {
                        var start = proxy.__observer__.changeset.length === 0,
                            deliver = proxy.__observer__.deliver();

                        proxy.__observer__.changeset.push(change);

                        if (start) {
                            deliver(false, (typeof(deliver.delay)==="number" ? deliver.delay : 10));
                        }
                    }
                }
            }

            if (property==="push") {
                 return function(item) {
                    return this.splice(this.length, 0, item);
                }
            }

            if (property==="pop") {
                 return function() {
                    return this.splice(this.length-1, 1);
                }
            }

            if (property==="unshift") {
                 return function(item) {
                    return this.splice(0, 0, item);
                }
            }

            if (property==="shift") {
                return function() {
                    return this.splice(0, 1);
                }
            }

            return target[property];
        }
    });

    var proxy = Object.observe(arrayproxy, function(changeset){ 
        var changes = changeset.filter(function(change){
            return change.name!=="length" && change.name!=="add" && (!acceptlist || acceptlist.indexOf(change.type)>=0);
        });

        if (changes.length>0) {
            callback(changes);
        }
    }, acceptlist, pausable, pause, delay);
    return proxy;
};

Array.unobserve = function(object, callback) {
  return object.unobserve(callback);
};


Object.deepObserve = function(object,callback,parts) {
    var parts = parts || [];

    var toTypeName = function(obj) {
        return ({}).toString.call(obj).match(/\s([a-zA-Z]+)/)[1].toLowerCase()
    };

    function reobserve(value, parts) {
        var keys = Object.keys(value);

        keys.forEach(function(key) {
            if ((toTypeName(value[key]) === 'object' || toTypeName(value[key]) === 'array') && !value[key].hasOwnProperty('__observers__')) {
                var newparts = parts.slice(0);
                newparts.push(key);
                value[key] = Object.deepObserve(value[key], callback, newparts);
            }
        });
    }

    reobserve(object, parts);

    var observed = Object.observe(object, function(changeset){
        var changes = [];
        
        function recurse(name, rootObject, oldObject, newObject, path) {
            if (newObject instanceof Object) {
                var newkeys = Object.keys(newObject);

                newkeys.forEach(function(key){
                    if (!oldObject || (oldObject[key]!==newObject[key])) {
                        var oldvalue = (oldObject && oldObject[key]!==undefined ? oldObject[key] : undefined),
                            change = (oldvalue===undefined ? "add" : "update"),
                            keypath = path + "." + key;

                        changes.push({
                            name: name,
                            object: rootObject,
                            type: change,
                            oldValue: oldvalue,
                            newValue: newObject[key],
                            keypath: keypath
                        });

                        recurse(name, rootObject, oldvalue, newObject[key], keypath);
                    }
                });
            } else if (oldObject instanceof Object) {
                var oldkeys = Object.keys(oldObject);

                oldkeys.forEach(function(key) {
                    var change = (newObject===null ? "update" : "delete"),
                        keypath = path + "." + key;
                        
                    changes.push({
                        name: name,
                        object: rootObject,
                        type: change,
                        oldValue: oldObject[key],
                        newValue: newObject,
                        keypath: keypath
                    });
                    
                    recurse(
                        name,
                        rootObject,
                        oldObject[key],
                        undefined,
                        keypath
                    );
                });
            }
        }

        changeset.forEach(function(change) {
            var keypath = (parts.length>0 ? parts.join(".") + "." : "") + change.name;

            if (change.type === "update" || change.type === "add") { 
                reobserve(change.object, parts);
            }

            changes.push({
                name: change.name,
                object: change.object,
                type: change.type,
                oldValue: change.oldValue,
                newValue: change.object[change.name],
                keypath: keypath
            });

            recurse(
                change.name,
                change.object,
                change.oldValue,
                change.object[change.name],
                keypath
            );
        });

        callback(changes);
    });
    return observed;
};

var check = function(key, action){
    if (key[0] === '_') {
        throw new Error(`Invalid attemp to ${action} private "${key}" property`);
    }
};

var handler = {
    has(target, key) {
        return key[0] === '_' ? false : key in target;
    },

    get(target, key) {
        console.log("get", key);
        check(key, "get");
        return target[key];
    },

    set(target, key, value) {
        console.log("set", key, value);
        check(key, "set");
        return true;
    },
    
    deleteProperty(target, key) {
        console.log("delete", key);
        check(key, "delete");
        return false;
    },

    defineProperty(target, key, descriptor) {
        console.log("define", key);
        check(key, "define");
        return true;
    },

    enumerate(target) {
        return Object.keys(target).filter(key=>key[0]!=='_')[Symbol.iterator]();
    }
};

var Proxenet = function(o, callback){
    var self = this,
        hash = new WeakMap(),
        activate = false;

    var handler = function(changes){
        if (activate) callback(changes);
    };

    var walk = function(object){
        var proxy;

        if (object){
            
            if (object.constructor == Array){
                console.log("parsing array", object);
                proxy = Array.observe(object, handler);
            }

            else if (object.constructor == Object){
                hash.set(object, true);

                proxy = Object.observe(object, handler);
                console.log("parsing object", object);

                for (var i in object) {
                    proxy[i] = walk(object[i]);
                }

            }

            else {
                return object;
            }
        }

        return proxy;
    };

    var out = walk(o);
    activate = true;

    return out;
};

var observer = {
    create : function(o, callback){
        o = Proxenet(o, callback);
        return o;
    }
};

module.exports = observer;