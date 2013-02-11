/* ------------------------+------------- */
/* Native Preloader 1.0    | Falcon Build */
/* ------------------------+------------- */

var document = {
    body: Native.canvas
};

document.createElement = function(type) {
    switch(type) {
        case "canvas":
            return new Canvas(32, 32);
        default:
            break;
    }
}

Canvas.prototype.appendChild = function(node) {
    this.add(node);
}

if ("titleBarColor" in window) {
	window.titleBarColor = "rgba(34, 35, 33, 1)";
}

if ("opacity" in window) {
	//window.opacity = 0.95;
}

if ("titleBarControlsOffsetY" in window) {
	window.titleBarControlsOffsetX = 0;
	window.titleBarControlsOffsetY = 3;
}
