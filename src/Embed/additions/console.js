/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
{

    console.lazylog = function(m){
        setTimeout(function(){
            console.log(m);
        }, 3000);
    };

    console.dump = function(...n){
        this.iteration = 0;
        this.maxIterations = 5000;
        for (var i in n){
            if (n.hasOwnProperty(i)) {
                console.log(
                    console.parse(n[i])
                );
            }
        }
    };

    console.parse = function(object){
        var self = this,
            visited = [],
            circular = false;

        var dmp = function(object, pad){
            var out = '',
                idt = '\t';

            circular = false;

            for (i = 0; i < visited.length; i++) {
                if (object === visited[i]) {
                    circular = true;
                    break;
                }
            }

            self.iteration++;
            if (self.iteration>self.maxIterations){
                return false;
            }

            pad = (pad === undefined) ? '' : pad;

            if (circular) {
                out = '[circular reference]';
            } 

            else if (object === null){
                out = 'null';
            } 

            else if (object != null && object != undefined){
                
                if (object.constructor == Array){
                    out += '[';
                    if (object.length>0){
                        var arr = [];
                        out += '\n';
                        for (var i=0; i<object.length; i++){
                            arr.push(pad + idt + dmp(object[i], pad + idt));
                        }
                        out += arr.join(',' + '\n') + '\n';
                        out += pad;
                    }
                    out += ']';
                } 

                else if (object.constructor == Object){
                    out += '{\n';
                    visited.push(object);
                    for (var i in object){
                        out += pad + idt + i + ' : ' 
                            + dmp(object[i], pad + idt) + '\n';
                    }
                    out += pad + '}';
                } 

                else if (typeof(object) == "string"){
                    out += '"' + object + '"';
                } 

                else if (typeof(object) == "number"){
                    out += object.toString();
                } 

                else if (object.constructor === Function){
                    visited.push(object);
                    var source = object.toString();
                    if (source.indexOf('[native code]') > -1) {
                        out += "function(){ [native code] }";
                    } else {
                        out += "function(){ ... }"; //source;
                    }

                } 

                else if (object.toString) {
                    try {
                        out += object;
                    } catch(e){
                        out += "function(){ [Native Code] }";
                    }
                } else {
                    out += "null";
                }
            } else {
                out += 'undefined';
            }
            return out;
        };

        self.iteration = 0;
        return dmp(object);
    };

}
