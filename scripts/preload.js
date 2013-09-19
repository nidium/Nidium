/* --------------------------------------------------------------------------- *
 * Native Minimal Built'in FrameWork                       (c) 2013 Stight.com * 
 * --------------------------------------------------------------------------- * 
 * Version:     1.0                                                            *
 *                                                                             *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * and/or sellcopies of the Software, and to permit persons to whom the        *
 * Software is furnished to do so, subject to the following conditions:        *
 *                                                                             *
 * The above copyright notice and this permission notice shall be included in  *
 * all copies or substantial portions of the Software.                         *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
 * DEALINGS IN THE SOFTWARE.                                                   *
 * --------------------------------------------------------------------------- * 
 */

/*
Les grands orateurs qui dominent les assemblées par l’éclat de leur parole sont,
en général, les hommes politiques les plus médiocres : il ne faut pas les
combattre par des paroles, ils en ont toujours de plus ronflantes que les vôtres
; il faut opposer à leur faconde un raisonnement serré, logique ; leur force est
dans le vague, il faut les ramener dans la réalité des faits : la pratique les
tue.
*/

document.body = window.canvas;

document.createElement = function(type){
	switch (type){
		case "canvas":
			return new Canvas(32, 32);
			default:
			break;
	}
};

document.location = {
	set href(val) {
		document.run(val);
	}
}

Canvas.prototype.appendChild = function(node) {
	this.add(node);
};

/*

if ("titleBarColor" in window) {
	window.titleBarColor = "rgba(255, 255, 255, 0.1)";
}

if ("opacity" in window) {
	window.opacity = 0.95;
}

if ("titleBarControlsOffsetY" in window) {
	window.titleBarControlsOffsetX = 0;
	window.titleBarControlsOffsetY = 0;
}

*/

/*
var close_png = "data:image.png;base64,iVBORw0KGgoAAAANSUhEUgAAAAwAAAANCAYAAACdKY9CAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH3QISAA8kWT+ROwAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAACiElEQVQoFQF9AoL9AeLi4v8BAQEA8enpANO/vwDczswA5tzcAAAAAAAaJCQAIzIzAC1AQQAQGBgAAAAAAAHi4uL/4NLSAMempQDizckAHRMTAB4XGAD///8A4unpAOPt7QAeNDYAOVlbACEvLwAE7ubmAMimpQDuz8wAcUhHABtDQgAGOzoAAP8AAPrFxgDkvb0AnLq9AMoAAQAQFhYAAz8lJYDhtLEAVSonACEuLQAB8PAA/M7PAP3Q0AAB8vAADA8OABf19QDN0NEAAhIUAATt0M0ALAsJABf9+wD81tUABA0PAAIICAAAAAAA/vj4APzz8QAX/vsAKwoJAO3PzgAE+eDgACUKBwAM9/YACO3sAAH49wAABwoAAAAAAAP59gAE9vYA//j3ACUKBQDE4t8ABAkGBAAOBgYACQwMAAELCgAACQsAAAcHAAAAAQAAAwIAAQICAAkBAAAPAAEACPj9AAQOJCUA8ujnAAAPEAAAEA8AABETAAEODgAAAQEA//PzAADv7wAABAUA8vwAAA0dFwAEEC0vAOju7gAmExYAChEQAAEaGgABEhQAAAEBAADu7AD/AwYA9wQGAOjw9gATLjEAAhs3OAD6IiMAz/j6APYUFwD+EBMAAAwOAAAMDgD+ERQA9hYWAM/5+gD6ISEAHjo6AAHc3Nz/8N/fAObCwAAC9/YAGiQlABQYGQAAAQAA7OfoAObd2wD9BwYAHEBEAA4fHwAB19fX/wUFBQD+9fUA6tTRAO/b3ADx5+cAAAAAAA8ZGQASJiUAFCotAAMMDAD8/PwAAdPT0/8CAgIABgYGAAgICAAGBgYAAwMDAAAAAAD9/f0A+vr6APj4+AD7+/sA/v7+ADUWzIHPE/LYAAAAAElFTkSuQmCC====";
var min_png = "data:image.png;base64,iVBORw0KGgoAAAANSUhEUgAAAAwAAAANCAYAAACdKY9CAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH3QISABAKSLOSagAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAACiElEQVQoFQF9AoL9AePj4/8AAAAA7+vnANfOwADh2MoA6eDaAAAAAAAWICYAICg2ACkyQAARFRkAAAAAAAHh4eH/4trRAMq7pADr08IAHxkQAB8dGgAAAAAA4ePoAOHn7gAVLT4AN0ZdAB4mLwAE8OzoAM29pAD92b8AamhNABU6SwAGD0AAAAABAPvwwADqyLQAqJ2zAMwCAAANERYAA0I2JIDux6cAUEgqABc1OwAB+/cA/O7OAP3w0AAA/PUACRISABsE7wDWycwA/A0ZAATu4s4ALB0KAA4LCAD84NgAAwwPAAIGCQAAAAAA/fv4AP308QAOCwgALB0KAODhzQAE+PTgABodEgAHCAAABAD1AAL9AAAC+/sAAAAAAAH89gAB//UABwgAABodEgDZ5+EABAcIBQAGEQgABBAOAAUODgACBwkAAQQIAAAAAAD//AIA/gcDAP4C/wAGEfcAB+/9AAQNFCMA8f7qAB4KDQACCg0AAgoNAAAFDQAAAAAAAfvzAAAFBAAC/fsA8gAJAA0UFwACERouAOv5+wD5BAMAAAkFAAAHDQAACREAAAkRAAAIDQAABwUA+QQCAOr3/AARGi4AAhwmOgD5BSUA09/pAPT7/AD7//4A/v/9AP7//QD7/v4A9Pv8ANPe6wD5BSUAHSc7AAHa2tr/8OzeAObXtAAHBOEAGR0WABQWGAAAAAAA7OroAOfj6gD5/B8AGilLABAUIwAB2NjY/wUFBQD8+fIA6uLJAPDo1ADz8uAAAAAAAAwNHwARGS4AFR01AAIFDAD///8AAdbW1v8AAAAABAQEAAkJCQAGBgYAAQEBAAEBAQD+/v4A+/v7APf39wD9/f0A/f39AO1jz64BdGYUAAAAAElFTkSuQmCC====";
var zoom_png = "data:image.png;base64,iVBORw0KGgoAAAANSUhEUgAAAAwAAAANCAYAAACdKY9CAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH3QISABAgkwhbvAAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAACiElEQVQoFQF9AoL9AeTk5P/+/v4A7O7rAMjQwQDW3dAA3+PeAAAA/wAgHSMAKSEuADkxQAAXFRgAAAAAAAHj4+P/1trRALTAqwDS4M4AFh8TABocFgD///4A5ePoAOvi8AAsIDIAUEJXACcjLAAE5+rmALLAqQDY6tIAX2lRAEUpSAAVDC4AAAAAAOn0zwC51rUAp6G2AALAAABdDhMAAzE5KIDC2LgAPUswADcqNQDzAPIA4PTTAOT21gD3APUAFQ4TAPwM8wDNz9AAEgcTAAMkNBuAABj3AB8hGAD2/PYA9/r4APv9+QD4+/YA7vTtAOXw5QD8APsA9Pv1APft/AAE7vXlABopFAABCf8A+gH4APb+/wD7+wYAAAAAAPv9+gD6AfgAAgj+ABkeFgDrzeUABAgJBQAPDA0ADAkLAAwICQAIAwYABwMIAP///wD6/gIAAwUCAAAD/wAPDfsA8gn4AAQYEyAA/vTzAAgDBwANBgkADQUPAAoDDAAAAAAA9v30APH68AD8/PwA/vX/ABkRHwACIxcqAPju+gAH/gUACgEKAA8FDwAPBxMADwcTAA4FDwALAgoAB/0EAPjs+gAiGCkAAi0mNQASAx0A49frAP/5AQAC/QQAAv8DAAL/AwAC/QQA//kBAOPX6wASAh4ALCU1AAHa2tr/6O7iAM7bvgD5/usAHxsgABYWFgAAAAAA6erpAODk4AAIARQAMSVCABwWIgAB2dnZ/wMDAwD2+vMA3uXTAOHp2ADu7+kAAAD/ABMRGQAdFiYAJB0vAAoGDQD5+fkAAdTU1P8CAgIAAwMDAAoKCgAFBQUABAQEAP///wD9/f0A+/v7APb29gD7+/sAAQEBADs05sgFaDe9AAAAAElFTkSuQmCC====";

var ctx = Native.canvas.ctx;
ctx.fillStyle = "red";
ctx.fillRect(0, 0, 100, 100);

var ctxx = Native.titleBar.ctx;

var gradient = ctxx.createLinearGradient(0, 0, 0, 35);
gradient.addColorStop(0, '#F1F1F1');
gradient.addColorStop(0.035, '#e7e7e7');
gradient.addColorStop(1, "#BFBFBF");

ctxx.fillStyle = gradient;
ctxx.fillRect(0, 0, Native.titleBar.width, Native.titleBar.height);

var close = new Image();
close.src = close_png;
close.onload = function() {
    ctxx.drawImage(close, 10, 10);
}

var min = new Image();
min.src = min_png;
min.onload = function() {
    ctxx.drawImage(min, 30, 10);
}

var zoom = new Image();
zoom.src = zoom_png;
zoom.onload = function() {
    ctxx.drawImage(zoom, 50, 10);
}
*/