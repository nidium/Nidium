/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{
    const Elements = require("Elements");

    document.getElementById = function(id){
        return document.canvas.shadowRoot.getElementById(id);
    };

    document.createElement = function(name) {
        return Elements.Create(name);
    }
}
