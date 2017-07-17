/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

var drawer = {
    setShadow : function(ctx, style){
        ctx.shadowOffsetX = style.shadowOffsetX;
        ctx.shadowOffsetY = style.shadowOffsetY;
        ctx.shadowColor = style.shadowColor;
        ctx.shadowBlur = style.shadowBlur;
    },

    disableShadow : function(ctx){
        this.setShadow(ctx, {
            shadowOffsetX : 0,
            shadowOffsetY : 0,
            shadowColor : 0,
            shadowBlur : 0
        });
    },

    roundbox : function(ctx, bounds, radius, fill, stroke, lineWidth) {
        if (!stroke && !fill) return false;

        if (fill) {
            ctx.fillStyle = fill;
            ctx.fillRect(bounds.x, bounds.y, bounds.w, bounds.h, radius);
        }

        if (stroke && lineWidth) {
            ctx.lineWidth = lineWidth;
            ctx.strokeStyle = stroke;

            ctx.strokeRect(
                bounds.x-lineWidth/2,
                bounds.y-lineWidth/2,
                bounds.w + lineWidth,
                bounds.h + lineWidth,
                radius + lineWidth/2
            );
        } 
    }
};

module.exports = drawer;