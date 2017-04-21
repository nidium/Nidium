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
load("embed://framework/elements/li.js");
load("embed://framework/elements/div.js");
load("embed://framework/elements/view.js");
load("embed://framework/elements/span.js");
load("embed://framework/elements/img.js");
load("embed://framework/elements/section.js");
load("embed://framework/elements/radio.js");

/*
 * Grid layout
 */
load("embed://framework/elements/grid/grid.js");
load("embed://framework/elements/grid/row.js");
load("embed://framework/elements/grid/col.js");

/*
 * Native controllers (medium level)
 */
load("embed://framework/elements/navigator.js");


/*
 * Native components (high level)
 */
require("./components/button.nc");
require("./components/icon.nc");
require("./components/statusbar.nc");
require("./components/listview.nc");

/**
 * Fonts
 */
window.fontShapes = require('./fonts/fontawesome.js');
