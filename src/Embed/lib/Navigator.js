/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = require("Elements");

const __opacity_lo__ = 0.4;
const __opacity_hi__ = 1.0;
const __next_duration__ = 450;
const __back_duration__ = 420;

const { ElementStyle } = require("ElementsStyles");

class Navigator extends Elements.Element {
    constructor(attributes) {
        super(attributes);
        this.routes = {};
        this.history = [];

        this.style.flexGrow = 1;
    }

    set(routes) {
        this.routes = {};

        for (var url in routes) {
            if (routes.hasOwnProperty(url)) this.routes[url] = routes[url];
        }
    }

    go(url, params) {
        var component = this.routes[url];
        if (!component) return false;

        for (var i = 0, len = this.history.length; i < len; i++) {
            // if ()
        }

        var scene = new component(params);
        scene.createTree();
        scene.url = url;
        scene.navigator = this;

        this.push(scene);
    }

    reset() {
        this.empty();
        this.history = [];
    }

    push(nextScene) {
        if (this.history.length > 0) {
            var currScene = this.history[this.history.length - 1];
            currScene.opacity = __opacity_hi__;
            setAnimation(
                c => {
                    c.left = -window.innerWidth;
                    c.opacity = __opacity_lo__;
                },
                __next_duration__,
                Easing.Exponential.Out,
                currScene
            );

            nextScene.left = window.innerWidth;
            nextScene.opacity = __opacity_lo__;
            setAnimation(
                c => {
                    c.left = 0;
                    c.opacity = __opacity_hi__;
                },
                __next_duration__,
                Easing.Exponential.Out,
                nextScene
            );
        }

        this.history.push(nextScene);
        this.add(nextScene);
    }

    back(callback) {
        var callback = callback || function() {};

        if (this.history.length <= 1) return false;

        var currScene = this.history.pop();
        currScene.opacity = __opacity_hi__;

        var anim = setAnimation(
            c => {
                c.left = window.innerWidth;
                c.opacity = __opacity_lo__;
            },
            __back_duration__,
            Easing.Exponential.Out,
            currScene
        );

        anim.onFinish = () => {
            currScene.removeFromParent();
            callback.call(this);
        };

        var prevScene = this.history[this.history.length - 1];
        prevScene.left = -window.innerWidth;
        prevScene.opacity = __opacity_lo__;

        setAnimation(
            c => {
                c.left = 0;
                c.opacity = __opacity_hi__;
            },
            __back_duration__,
            Easing.Exponential.Out,
            prevScene
        );
    }
}

ElementStyle.Inherit(Navigator);

module.exports = Navigator;