/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

{
    let MapFunc = new Map([
        [WebGLRenderingContext.FLOAT, "uniform1f"],
        [WebGLRenderingContext.FLOAT_VEC2, "uniform2fv"],
        [WebGLRenderingContext.FLOAT_VEC3, "uniform3fv"],
        [WebGLRenderingContext.FLOAT_VEC4, "uniform4fv"],
        [WebGLRenderingContext.INT, "uniform1i"],
        [WebGLRenderingContext.INT_VEC2, "uniform2iv"],
        [WebGLRenderingContext.INT_VEC3, "uniform3iv"],
        [WebGLRenderingContext.INT_VEC4, "uniform4iv"],
    ]);

    CanvasRenderingContext2D.prototype.setShader = function(file) {
        var shader = File.readSync(file, {encoding: "utf8"});

        if (!shader) {
            return;
        }

        var program = this.attachFragmentShader(shader);
        var uniforms = program.getActiveUniforms();
        var ret = { __props: {} };

        for (let i of uniforms) {
            let { name, type, location } = i;
            let prop = {
                configurable: true,
                set: function(value) {
                    throw new Error("Unsupported uniform type");
                    return false;
                },
                get: function(prop) {
                    return ret.__props[name] || 0;
                }
            }

            let func = MapFunc.get(type);
            if (func) {
                prop.set = (value) => {
                    program[func](location, value);
                    ret.__props[name] = value;
                }
            }

            Object.defineProperty(ret, name, prop);
        }

        return ret;
    }
}