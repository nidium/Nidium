/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

/* -------------------------------------------------------------------------- */

DOMElement.implement({
	shader : function(url, callback){
		var self = this;

		File.getText(url, function(source){
			var	uniforms = {},
				ctx = self.layer.context,
				program = ctx.attachGLSLFragment(source);

			var setUniformValue = function(location, value){
				program.uniform1i(location, value);
			};

			var createUniformBinding = function(uniform){
				var name = uniform.name,
					location = uniform.location,
					type = uniform.type;

				Object.defineProperty(uniforms, name, {
					configurable : false,
					get : function(){
						return uniform.value ? uniform.value : 0;
					},

					set : function(value){
						uniform.value = value;
						setUniformValue(location, value);
					}
				});
			};

			var uniformArray = program.getActiveUniforms();

			for (var i=0; i<uniformArray.length; i++){
				createUniformBinding(uniformArray[i]);
			}

			if (typeof callback == "function") {
				callback.call(self, program, uniforms);
			}

		});

	}
});

/* -------------------------------------------------------------------------- */
