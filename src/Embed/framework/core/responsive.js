/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

"use strict";

var responsive = function(){
    const w = window.innerWidth;

    if (w<350){
        return 'xs';
    }

    if (w>=350 && w<768){
        return 'sm';
    }

    if (w>=768 && w<1024){
        return  'md';
    }

    if (w>=1024 && w<1280){
        return 'lg';
    }

    if (w>=1280){
        return 'xl';
    }
};

module.exports = responsive;