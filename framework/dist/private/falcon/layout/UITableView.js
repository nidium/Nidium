/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

/* -------------------------------------------------------------------------- */
/* NSS PROPERTIES                                                             */
/* -------------------------------------------------------------------------- */

document.nss.add({
	"UITableView" : {
		overflow : false,
		scrollable : true,
		scrollBarX : true,
		scrollBarY : true
	},

	"UITableHead" : {
		height : 40,
		color : null,
		background : "rgba(0, 0, 0, 0.8)"
	},

	"UITableBody" : {
	},

	"UITableRow" : {
		height : 20,
		fontSize : 11,
		color : null,
		background : null
	},

	"UITableRow:even" : {
	},

	"UITableRow:selected" : {
		color : "#ffffff",
		background : "#2277E0"
	},

	"UITableRow:disabled" : {
		background : "#bbbbbb",
		color : "#999999",
		cursor : "arrow"
	},

	"UITableRow:disabled+hover" : {
		background : "#aaaaaa",
		cursor : "arrow"
	}
});

/* -------------------------------------------------------------------------- */
/* ELEMENT DEFINITION                                                         */
/* -------------------------------------------------------------------------- */

Native.elements.extend("UIView").export("UITableView", {
	onAdoption : function(){
		/* overide UIView onAdoption with nothing to do */
	},

	update : function(e){
		if (e.property == "scrollTop" && this.head.fixed === true) {
			this.head.top = this.scrollTop;
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this._topPosition = 0;
		this._nbrows = 0;

		/* Element's Dynamic Properties */
		NDMElement.defineDynamicProperties(this, {
			even : false
		});

		this.body = new UITableBody(this);
		this.body.rows = [];

		this.head = new UITableHead(this);

		this.addRow = function(options){
			var row = new UITableRow(this.body, {
				top : this._topPosition,
				id : options.id
			});

			row.even = (this._nbrows++) % 2 ? false : true;
			row.controller = this;
			this._topPosition += row.height;

			this.body.height = this._topPosition;
			this.body.rows.push(row);
			return row;
		};

		this.setHead = function(cells){
			var left = 0;
			this.head.cells = [];
			
			this.body.top = this.head.height;
			this.body.height -= this.head.height;

			for (var i=0; i<cells.length; i++){
				var cell = cells[i];

				this.head.cells[i] = new UILabel(this.head, {
					top : 0,
					left : left,
					autowidth : false,

					label : cell.label,
					width : cell.width,
					textAlign : cell.textAlign || this.head.textAlign,

					color : cell.color || this.head.color,
					fontSize : cell.fontSize || this.head.fontSize,
					fontFamily : cell.fontFamily || this.head.fontFamily,
					background : cell.background || this.head.background,

					height : this.head.height
				});

				left += parseInt(cells[i].width, 10);
			}
			this.head.width = left;
			this.body.width = left;
			return this.head;
		};

	}
});

Native.elements.extend("UIElement").export("UITableHead", {
	init : function(){
		this.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("drag", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("dragend", function(e){
			e.forcePropagation();
		}, true);
	},
	
	draw : function(context){
		var	params = this.getDrawingBounds();

		NDMElement.draw.box(this, context, params);
		NDMElement.draw.glassLayer(this, context, params);
	}
});

Native.elements.extend("UIElement").export("UITableBody", {
	init : function(){
		this.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("drag", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("dragend", function(e){
			e.forcePropagation();
		}, true);
	}
});


Native.elements.extend("UIElement").export("UITableRow", {
	onAdoption : function(){
		/* overide UIElement onAdoption with nothing to do */
	},

	update : function(e){
		if (e.property.in("selected", "hover", "disabled")) {
			this.updateCells();
		}
	},

	init : function(){
		var self = this,
			o = this.options;

		this.cells = [];
		this.overflow = false;

		NDMElement.listeners.addHovers(this);

		this.updateCells = function(){
			for (var i=0; i<this.cells.length; i++){
				this.cells[i].color = this.color;
			}
		};

		this.addCell = function(options){
			var table = this.controller,
				index = this.cells.length,
				headCell = table.head.cells[index];

			if (!headCell) throw "Table Head mismatch";

			var cell = new UILabel(this, {
				top : 0,
				left : headCell.left,
				height : this.height,
				width : headCell.width,
				autowidth : false,

				id : options.id,
				label : options.label || "",

				textAlign : options.textAlign || this.textAlign,
				fontSize : options.fontSize || this.fontSize,
				fontFamily : options.fontFamily || this.fontFamily,
				color : options.color || this.color
			});

			this.cells.push(cell);

			return cell;
		};

		this.addEventListener("dragstart", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("drag", function(e){
			e.forcePropagation();
		}, true);

		this.addEventListener("dragend", function(e){
			e.forcePropagation();
		}, true);

	}
});
