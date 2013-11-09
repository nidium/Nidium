/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

document.backgroundImage = "private://assets/patterns/egg_shell.png";


document.nss.add({
	".input" : {width:60},

	".buy" : {
		top : 20
	},

	".fb" : {
		top : 50
	},

	".sell" : {
		top : 100
	},

	".fs" : {
		top : 130
	},

	".lab" : {
		textAlign : "right",
		paddingRight : 10,
		radius : 4,
		autowidth : false,
		width : 220,
		background : "rgba(255, 255, 255, 0.8)",
		fontSize : 15,
		color : "#880000"
	},

	".gain" : {
		top : 170,
		color : "white",
		background : "black",
		fontSize : 16,
		height : 34
	},

	".roi" : {
		top : 210,
		background : "black",
		fontSize : 26,
		height : 34
	},

	".pane" : {
		left : 10,
		width : 480,
		height : 70,
		radius : 8
	},

	".bbb" : {
		background : "rgba(0, 50, 0, 0.15)",
		top : 10
	},

	".sss" : {
		background : "rgba(100, 0, 0, 0.15)",
		top : 90
	}

});

var label = function(options){
	return new UILabel(document, options);
};

var input = function(options){
	return new UITextField(document, options);
};

/* PANES */

var p1 = new UIElement(document, "pane bbb");
var p2 = new UIElement(document, "pane sss");

/* LABELS */

var label_buy = label({left:20, label:"BUY", class:"buy"});
var label_fb = label({left:260, label:"-0.0 BTC (0 EUR)", class : "lab fb"});
var label_sell = label({left:20, label:"SELL", class : "sell"});
var label_fs = label({left:260, label:"-0.0 BTC (0 EUR)", class : "lab fs"});
var label_gain = label({left:260, label:"RETURN: 0 EUR", class:"lab gain"});
var label_roi = label({left:260, label:"ROI: 0 %", class : "lab roi"});

var label_BTC1 = label({left:124, label:"X", class:"buy"});
var label_BTC2 = label({left:124, label:"X", class:"sell"});

var label_EUR1 = label({left:210, label:"EUR", class:"buy"});
var label_EUR2 = label({left:210, label:"EUR", class:"sell"});

var label_total_buy = label({left:260, label:"INVEST: 0 EUR", class:"lab buy"});
var label_total_sell = label({left:260, label:"TOTAL: 0 EUR", class:"lab sell"});

var label_fees = label({left:10, top:180, label:"FEES >>>"});

/* INPUTS */

var input_nb_buy = input({left:56, width:60, placeholder:"Nb", class:"input buy"});
var input_buy_price = input({left:140, placeholder:"Price", class:"input buy"});

var input_nb_sell = input({left:56, width:60, placeholder:"Nb", class:"input sell"});
var input_sell_price = input({left:140, placeholder:"Price", class:"input sell"});

var input_fees_percent = input({left:70, top:180, width:35, placeholder:"Fees", class:"input", value:"0.6"});

input_nb_sell.editable = false;


/* LOCAL STORAGE */

var storeLocal = function(){
	window.storage.set("inputs", {
		nb_buy : input_nb_buy.value,
		buy_price : input_buy_price.value,
		fees : input_fees_percent.value
	});	
};

var getLocal = function(){
	var store = window.storage.get("inputs");
	if (!store) return false;

	input_nb_buy.value = store.nb_buy;
	input_buy_price.value = store.buy_price;
	input_fees_percent.value = store.fees;
	updateSecretFormula();
};

/* EVENTS */

input_nb_buy.addEventListener("change", function(e){
	updateSecretFormula();
	storeLocal();
});

input_buy_price.addEventListener("change", function(e){
	updateSecretFormula();
	storeLocal();
});

input_sell_price.addEventListener("change", function(e){
	updateSecretFormula();
});

input_fees_percent.addEventListener("change", function(e){
	updateSecretFormula();
	storeLocal();
});

var updateSecretFormula = function(){

	var nb_buy = Number(input_nb_buy.value) || 0,
		nb_sell = Number(input_nb_sell.value) || 0,
		buy_price = Number(input_buy_price.value) || 0,
		sell_price = Number(input_sell_price.value) || 0,
		fees = Number(input_fees_percent.value) || 0,

		fees_buy_nb = Math.round(10000000 * nb_buy * fees/100) / 10000000,
		fees_buy_price = Math.round(10000*fees_buy_nb * buy_price)/10000;

	input_nb_sell.value = nb_buy - fees_buy_nb;

	var	total_buy_price = Math.round(10000*nb_buy * buy_price)/10000,
		total_sell_price = Math.round(10000*nb_sell * sell_price)/10000,

		fees_sell_price = Math.round(10000*total_sell_price*fees/100)/10000,

		total_fees = fees_buy_price + fees_sell_price,
		gain = Math.round(100*((total_sell_price - total_buy_price) - fees_sell_price))/100,
		roi = total_buy_price!=0 ? Math.round(100*gain*100/total_buy_price)/100 : 0;


	label_fb.label = "-" + fees_buy_nb + " BTC (" + fees_buy_price + " EUR)";
	label_fs.label = "-" + fees_sell_price + " EUR";
	label_total_buy.label = "INVEST: "+total_buy_price+" EUR";
	label_total_sell.label = "TOTAL: "+total_sell_price+" EUR";
	label_gain.label = "RETURN: "+gain+" EUR";
	label_roi.label = "ROI: "+roi+" %";
	label_roi.background = "#000000";
	label_roi.color = roi>0 ? "#00BB00" : "#AA0000";
};

var setCurrentBTCPrice = function(json){
	if (json.result != "success") return false;

	var last_price = Math.round(1000 * json.data.last.value) / 1000,
		buy_price = Math.round(1000 * json.data.buy.value) / 1000,
		sell_price = Math.round(1000 * json.data.sell.value) / 1000;

	input_sell_price.value = last_price;
	updateSecretFormula();
};

/*
TODO : FIX "JSON.parse: unexpected character" in HTTP API
var g = '{"result":"success","data":{"last_local":{"value":"265.49428","value_int":"26549428","display":"265.49\u00a0\u20ac","display_short":"265.49\u00a0\u20ac","currency":"EUR"},"last":{"value":"265.49428","value_int":"26549428","display":"265.49\u00a0\u20ac","display_short":"265.49\u00a0\u20ac","currency":"EUR"},"last_orig":{"value":"226.50000","value_int":"22650000","display":"\u00a3226.50","display_short":"\u00a3226.50","currency":"GBP"},"last_all":{"value":"271.17629","value_int":"27117629","display":"271.18\u00a0\u20ac","display_short":"271.18\u00a0\u20ac","currency":"EUR"},"buy":{"value":"265.13109","value_int":"26513109","display":"265.13\u00a0\u20ac","display_short":"265.13\u00a0\u20ac","currency":"EUR"},"sell":{"value":"269.00000","value_int":"26900000","display":"269.00\u00a0\u20ac","display_short":"269.00\u00a0\u20ac","currency":"EUR"},"now":"1383984816107287"}}';
var m = JSON.parse(g);
setCurrentBTCPrice(m);
*/

var getCurrentBTCPrice = function(devise){
	devise = devise.toUpperCase();	

	document.status.label = "Loading BTC Price ...";
	document.status.open();

	var url = "http://data.mtgox.com/api/2/BTC"+devise+"/money/ticker_fast";

	var h = new HttpRequest('GET', url, null, function(e){
		document.status.label = "Complete";
		document.status.value = 0;
		document.status.close();
		if (e.data) setCurrentBTCPrice(e.data);
	});

	h.ondata = function(e){
		document.status.label = e.percent+"%";
		document.status.value = e.percent;
	};

	h.onerror = function(e){
		document.status.label = 'Error: ' + e.error;
		document.status.value = 0;
	};

};

getCurrentBTCPrice("EUR");
getLocal();