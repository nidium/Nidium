/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = require("Elements");

class Navigator extends Elements.Node {
    constructor(attributes) {
        super(attributes);

        this.scenes = [];
    }

    push(scene, params) {
        var instance = new scene(params);
        
        this.scenes.push(instance);
        document.canvas.add(instance);
    }

    back(scene) {
        var instance = this.scenes.pop();
        instance.removeFromParent();
    }
}

module.exports = Navigator;