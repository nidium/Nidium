/**
 * Copyright 2017 Nidium Inc. All rights reserved.
 * Use of this source code is governed by a MIT license
 * that can be found in the LICENSE file.
 */

{
	const Elements = require("Elements");

	Elements.section = class extends Elements.Element {
		constructor(attributes) {
			super(attributes);

			var mr = (min=50, max=250) => min + Math.floor(Math.random()*(max-min));
			this._color = `rgba(${mr(70, 100)}, ${mr(0, 200)}, ${mr(140, 210)}, 0.8)`;

			this.style.width = "150";
			this.style.height = "150";
		}

		paint(ctx) {
			ctx.fillStyle = this._color;
			ctx.fillRect(0, 0, this.width, this.height);
			ctx.strokeStyle = "rgb(0, 255, 255)";
			ctx.strokeRect(0.5, 0.5, this.width-1, this.height-1);

			if (this.computedAttributes.label) {
				ctx.fillStyle = "#000";
				ctx.textAlign = "center";
				ctx.fontSize = 20;
				ctx.fillText(
					this.computedAttributes.label, this.width / 2, this.height - 20
				);
			}
		}
	}
}
