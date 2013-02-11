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


var oldRequire = require;

