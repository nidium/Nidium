/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
    const Elements = require("Elements");

    Elements.button = class extends Elements.Element {
        constructor(attributes) {
            super(attributes);

            this.cursor = "pointer";
            this.position = "inline";

            this.on("mouseup", function(ev){
                console.log("WIP")
            });
        }

        paint(ctx) {
            ctx.fillStyle = "#aaa";
            ctx.stokeStyke = "#111";

            ctx.fillRect(0, 0, this.width, this.height, 15, 15);
            ctx.strokeRect(0, 0, this.width-0.5, this.height-0.5, 15, 15);
        }
    }
}