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

    push(view, params) {
        var nextScene = new view(params);

        if (this.scenes.length>0) {
            var currScene = this.scenes[this.scenes.length-1];
            currScene.opacity = 0.95;

            setTimeout(()=>{
                AnimationBlock(500, Easing.Cubic.Out, (c)=>{
                    c.left = -window.innerWidth;
                    c.opacity = 0.5;
                }, currScene);
            }, 100);


            nextScene.left = window.innerWidth;
            nextScene.opacity = 0.8;
            setTimeout(()=>{
                AnimationBlock(600, Easing.Cubic.Out, (c)=>{
                    c.left = 0;
                    c.opacity = 1;
                }, nextScene);
            }, 0);
        }

        this.scenes.push(nextScene);
        document.canvas.add(nextScene);

    }

    back() {
        if (this.scenes.length<=1) return false;


            var currScene = this.scenes.pop();
            currScene.opacity = 1;

            setTimeout(()=>{
                AnimationBlock(300, Easing.Cubic.Out, (c)=>{
                    c.left = window.innerWidth;
                    c.opacity = 0.8;
                }, currScene);
            }, 100);

            var prevScene = this.scenes[this.scenes.length-1];
            prevScene.left = -window.innerWidth;
            prevScene.opacity = 0.1;
            setTimeout(()=>{
                AnimationBlock(400, Easing.Cubic.Out, (c)=>{
                    c.left = 0;
                    c.opacity = 1;
                }, prevScene);
            }, 0);


/*
        var currScene = this.scenes.pop();
        currScene.removeFromParent();
*/


    }
}

module.exports = Navigator;