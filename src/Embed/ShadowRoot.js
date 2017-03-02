const s_ShadowRoot  = require("./Symbols.js").ElementShadowRoot;
const Elements      = require("Elements");

class ShadowRoot {
    constructor(host, options={}) {
        this.host = host;
        // FIXME : Storing association with node instance will
        // prevent any node instance to be garbage collected
        this.idAssociation = {};
        this.tagAssociation = {};
        this.unique = {};
        this.name = options.name || undefined;

        this.jsScope = options.scope || null;
        this.nssList = [];
        this.nss = null;

        host[s_ShadowRoot] = this;

        if (this.jsScope) {
            // TODO : Override document with a proxy ?
        }
    }

    addTag(name, instance) {
        if (!this.tagAssociation[name]) {
            this.tagAssociation[name] = [];
        }

        this.tagAssociation[name].push(instance);
    }

    addNSS(nss) {
        this.nssList.push(nss);
        if (this.nss) {
            // If the NSS was already generated, refresh them with the new NSS
            this.computeNSS();
        }
    }

    addID(id, instance) {
        if (this.idAssociation[id] && this.idAssociation[id] != instance) {
            console.warn(`[WARNING] shadowRoot already have an element with id ${id}. Adding new element will override current element with same id`);
        }
        this.idAssociation[id] = instance;
    }

    rmTag(id) {
        if (this.tagAssociation[id]) {
            this.tagAssociation[id].splice(this.tagAssociation[id].indexOf(id), 1);
        }
    }

    rmID(id) {
        delete this.idAssociation[id];
    }

    rmNSS(nssInstance) {
        this.nssList.splice(this.nssList.indexOf(nssInstance), 1);

        // Compute NSS again with the removed NSS
        this.computeNSS();
    }

    rm(el) {
        for (let child of el.getChildren()) {
            this.rm(child);
        }

        if (el.id) {
            this.rmID(el.id);
        }

        this.rmTag(el.name());

        if (el instanceof Elements.nss) {
            this.rmNSS(el.nss);
        }

        // XXX : Is this really what we want ?
        el[s_ShadowRoot] = null;
    }

    add(el) {
        if (el[s_ShadowRoot] == this) {
            // Same ShadowRoot, nothing to do
            return;
        }

        if (!el.shadowRoot) {
            // Element is a child of another ShadowRoot, move
            // the element and it's children to this ShadowRoot.
            if (el[s_ShadowRoot]) {
                el[s_ShadowRoot].rm(el);

                for (let child of el.getChildren()) {
                    this.add(child);
                }
            }

            el[s_ShadowRoot] = this;
        }

        if (el.id) {
            this.addID(el.id, el);
        }

        this.addTag(el.name(), el);

        if (el instanceof Elements.nss) {
            this.addNSS(el.nss);
        }
    }

    findNodeById(id) {
        return this.idAssociation[id];
    }

    findNodesByTag(id) {
        return this.tagAssociation[id];
    }

    getElementById(id) {
        return this.findNodeById(id);
    }

    getElementsByTagName(id) {
        return this.findNodesByTag(id);
    }

    computeNSS() {
        this.nss = {};
        for (let nss of this.nssList) {
            Object.assign(this.nss, nss.eval());
        }
        Object.freeze(this.nss);
    }

    getNSS() {
        if (!this.nss) {
            this.computeNSS();
        }

        return this.nss;
    }

    getJSScope() {
        return this.jsScope;
    }

    nodeExists(name) {
        return findNodesByTag(name) ? true : false;
    }
}

module.exports = ShadowRoot;
