/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

const Elements = require("Elements");

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

    go(url, params){
        var component = this.routes[url];
        if (!component) return false;

        for (var i=0, len=this.history.length; i<len; i++) {
           // if ()
        }

        var scene = new component(params);
        scene.url = url;
        scene.navigator = this;

        this.push(scene);
    }

    reset(){
        this.empty();
        this.history = [];
    }

    push(nextScene) {

        if (this.history.length>0) {

            var currScene = this.history[this.history.length-1];
            currScene.opacity = 1.00;
            setTimeout(()=>{
                AnimationBlock(480, Easing.Cubic.Out, (c)=>{
                    c.left = -window.innerWidth;
                    c.opacity = 0.5;
                }, currScene);
            }, 100);


            nextScene.left = window.innerWidth;
            nextScene.opacity = 0.2;
            setTimeout(()=>{
                AnimationBlock(480, Easing.Cubic.Out, (c)=>{
                    c.left = 0;
                    c.opacity = 1;
                }, nextScene);
            }, 0);
        }

        this.history.push(nextScene);
        this.add(nextScene);
    }

    back(callback) {
        var callback = callback || function(){};

        if (this.history.length<=1) return false;

        var currScene = this.history.pop();
        currScene.opacity = 1;

        setTimeout(()=>{
            AnimationBlock(350, Easing.Exponential.InOut, (c)=>{
                c.left = window.innerWidth;
                c.opacity = 0.8;
            }, currScene)(()=>{
                currScene.removeFromParent();
                callback.call(this);
            });
        }, 100);

        var prevScene = this.history[this.history.length-1];
        prevScene.left = -window.innerWidth;
        prevScene.opacity = 0.1;
        setTimeout(()=>{
            AnimationBlock(480, Easing.Exponential.InOut, (c)=>{
                c.left = 0;
                c.opacity = 1;
            }, prevScene);
        }, 0);

    }
}

module.exports = Navigator;