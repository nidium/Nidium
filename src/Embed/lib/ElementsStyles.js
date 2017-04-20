class StyleContainer {
    static Create(forProto) {
        var sc = StyleContainer.protoStyle.get(forProto);
        if (!sc) {
            sc = function(elem) {
                this.elem = elem;
            };

            StyleContainer.protoStyle.set(forProto, sc);
        }

        return sc;
    }

    static Get(forProto) {
        return StyleContainer.protoStyle.get(forProto);
    }
}

StyleContainer.protoStyle = new WeakMap();

class ElementStyle {
    constructor(name, options) {
        this.name = name;
        this.isNative = options.isNative || false;
        this.inherit = options.inherit || false;
        this.repaint = options.repaint || false;
    }

    static Implement(onclass, elementstyle) {
        var inobj = StyleContainer.Create(onclass.prototype);

        Object.defineProperty(inobj.prototype, elementstyle.name, {
            enumerable: true,
            get: function() {
                if (elementstyle.isNative)
                    return elementstyle.readNative(this.elem);
                if (elementstyle.inherit)
                    return elementstyle.readInherit(this.elem);
                return elementstyle.readDefault(this.elem);
            },
            set: function(value) {
                if (elementstyle.isNative)
                    return elementstyle.writeNative(this.elem, value);
                if (elementstyle.inherit)
                    return elementstyle.writeInherit(this.elem, value);
                return elementstyle.writeDefault(this.elem, value);
            }
        });
    }

    static Inherit(onclass) {
        var inobj = StyleContainer.Create(onclass.prototype);
        var obj = onclass.prototype;

        while (obj) {
            let target = StyleContainer.Get(Object.getPrototypeOf(obj));
            if (!target) {
                return;
            }

            for (let prop in target.prototype) {
                Object.defineProperty(
                    inobj.prototype,
                    prop,
                    Object.getOwnPropertyDescriptor(target.prototype, prop)
                );
            }

            obj = Object.getPrototypeOf(obj);
        }
    }

    /*
        One function per read/write type to avoid runtime lookup
    */
    readNative(element) {
        return element[this.name];
    }

    readDefault(element) {
        if (!element._styleDefault) element._styleDefault = {};
        return element._styleDefault[this.name];
    }

    readInherit(element) {
        return element.inherit[`_Style_${this.name}`];
    }

    writeNative(element, value) {
        if (this.repaint) {
            element.requestPaint();
        }
        element[this.name] = value;
    }

    writeDefault(element, value) {
        if (this.repaint) {
            element.requestPaint();
        }
        if (!element._styleDefault) element._styleDefault = {};

        element._styleDefault[this.name] = value;
    }

    writeInherit(element, value) {
        if (this.repaint) {
            element.requestPaint();
        }

        element.inherit[`_Style_${this.name}`] = value;
    }
}

module.exports = {
    ElementStyle,
    StyleContainer
};