const ComponentLoader = require("ComponentLoader");

var registerComponent = function(name){
	new ComponentLoader(`embed://components/${name}.nml`);
};

registerComponent("button");
registerComponent("square");