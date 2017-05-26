/*
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

/*
 * Native NML tags (low level)
 */
load("embed://framework/elements/node.js");
load("embed://framework/elements/element.js");
load("embed://framework/elements/layout.js");
load("embed://framework/elements/nss.js");
load("embed://framework/elements/script.js");
load("embed://framework/elements/template.js");

load("embed://framework/elements/component.js");
load("embed://framework/elements/slot.js");

/*
 * Native UI elements (low level)
 */
load("embed://framework/elements/canvas.js");
load("embed://framework/elements/textnode.js");
load("embed://framework/elements/comment.js");
load("embed://framework/elements/nodeoverlay.js");
load("embed://framework/elements/li.js");
load("embed://framework/elements/label.js");
load("embed://framework/elements/fieldset.js");
load("embed://framework/elements/div.js");
load("embed://framework/elements/p.js");
load("embed://framework/elements/h1.js");
load("embed://framework/elements/view.js");
load("embed://framework/elements/span.js");
load("embed://framework/elements/img.js");
load("embed://framework/elements/section.js");

load("embed://framework/elements/radio.js");
load("embed://framework/elements/checkbox.js");


/*
 * Grid & layout
 */
load("embed://framework/elements/overlay.js");
load("embed://framework/elements/grid/grid.js");
load("embed://framework/elements/grid/row.js");
load("embed://framework/elements/grid/col.js");

/*
 * Native controllers
 */
require("Navigator");


/*
 * Native components (high level)
 */
require("./components/button.nc");
require("./components/icon.nc");
require("./components/statusbar.nc");
require("./components/listview.nc");
require("./components/spinner.nc");

/**
 * Fonts
 */

window.fontShapes = {};

const fontAwesome = require('./fonts/fontawesome.js');
const ionicons = require('./fonts/ionicons.js');

for (var i in fontAwesome) {
    window.fontShapes[i] = fontAwesome[i];
}

for (var i in ionicons) {
    window.fontShapes[i] = ionicons[i];
}

load("embed://framework/fonts/default.js");
