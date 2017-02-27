const s_ShadowRoot  = require("./Symbols.js").ElementShadowRoot;
const Elements      = require("./lib/Elements.js");

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

    rm(child) {
        let prevShadow = child[s_ShadowRoot];
        prevShadow.rmID(child.id);
        prevShadow.rmTag(child.name());

        if (child instanceof Elements.nss) {
            prevShadow.rmNSS(child.nss);
        }
    }

    add(child) {
        if (child[s_ShadowRoot]) {
            child[s_ShadowRoot].rm(child);
        }

        child[s_ShadowRoot] = this;

        this.addID(child.id);
        this.addTag(child.constructor.name);

        if (child instanceof Elements.nss) {
            this.addNSS(child.nss);
        }
    }

    findNodeById(id) {
        return this.idAssociation[id];
    }

    findNodesByTag(id) {
        return this.tagAssociation[id];
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
