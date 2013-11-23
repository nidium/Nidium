/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

"use strict";

document.backgroundImage = "private://assets/patterns/egg_shell.png";
var currencies = ["EUR", "USD"];
var currentDevise = currencies[0];

/* HELPERS ------------------------------------------------------------------ */

var label = function(options){
	return new UILabel(document, options);
};

var input = function(options){
	return new UITextField(document, options);
};

var r2 = function(n){ return Math.round(100*n)/100; };
var r3 = function(n){ return Math.round(1000*n)/1000; };
var r4 = function(n){ return Math.round(10000*n)/10000; };
var r7 = function(n){ return Math.round(10000000*n)/10000000; };

/* PANES -------------------------------------------------------------------- */

var p1 = new UIElement(document, "pane bbb");
var p2 = new UIElement(document, "pane sss");

/* LABELS ------------------------------------------------------------------- */

var label_buy = label({left:20, label:"BUY", class:"buy"});
var label_fb = label({left:260, class : "lab fb"});
var label_sell = label({left:20, label:"SELL", class : "sell"});
var label_fs = label({left:260, label:"-0.0 BTC (0 EUR)", class : "lab fs"});
var label_gain = label({left:260, label:"RETURN: 0 EUR", class:"lab gain"});
var label_roi = label({left:260, label:"ROI: 0 %", class : "lab roi"});

var label_BTC1 = label({left:124, label:"X", class:"buy"});
var label_BTC2 = label({left:124, label:"X", class:"sell"});

var label_CURR = label({left:210, label:"EUR", class:"sell"});

var label_total_buy = label({left:260, label:"INVEST: 0 EUR", class:"lab buy"});
var label_total_sell = label({left:260, label:"TOTAL: 0 EUR", class:"lab sell"});

var label_fees = label({left:10, top:180, label:"FEES >>>"});

/* INPUTS ------------------------------------------------------------------- */

var input_nb_buy = input({left:56, width:60, placeholder:"Nb", class:"input buy"});
var input_buy_price = input({left:140, placeholder:"Price", class:"input buy"});

var input_nb_sell = input({left:56, width:60, placeholder:"Nb", class:"input sell"});
var input_sell_price = input({left:140, placeholder:"Price", class:"input sell"});

var input_fees_percent = input({left:70, top:180, width:35, placeholder:"Fees", class:"input", value:"0.6"});

input_nb_sell.editable = false;

/* DROPDOWN ----------------------------------------------------------------- */

var	ddCurrency = document.add("UIDropDownController", {
	name : "currency",
	left : 205,
	width : 50,
	radius : 2,
	paddingLeft : 5,
	class : "buy"
});

ddCurrency.setOptions([
	{label : "EUR",	value : 0},
	{label : "USD",	value : 1}
]);

ddCurrency.addEventListener("change", function(e){
	storeLocal();
	setCurrency(e.value);
	getCurrentBTCPrice();
});

/* LOCAL STORAGE ------------------------------------------------------------ */

var storeLocal = function(){
	window.storage.set("inputs", {
		nb_buy : input_nb_buy.value,
		buy_price : input_buy_price.value,
		fees : input_fees_percent.value,
		currency : ddCurrency.value
	});	
};

var getLocal = function(){
	var store = window.storage.get("inputs");
	if (!store) return false;

	input_nb_buy.value = store.nb_buy;
	input_buy_price.value = store.buy_price;
	input_nb_sell.value = getNbAvailableForSell(store.nb_buy);
	input_fees_percent.value = store.fees;

	setCurrency(store.currency);
};

/* EVENTS ------------------------------------------------------------------- */

input_nb_buy.addEventListener("change", function(e){
	input_nb_sell.value = getNbAvailableForSell(this.value);
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

/* CALCULUS ----------------------------------------------------------------- */

var getNbAvailableForSell = function(nb){
	var nb_buy = Number(nb) || 0,
		fees = Number(input_fees_percent.value) || 0,
		fees_buy_nb = Math.round(10000000 * nb_buy * fees/100) / 10000000;

	return nb_buy - fees_buy_nb;
};

var updateSecretFormula = function(){
	var cc = currentDevise,
		nb_buy = Number(input_nb_buy.value) || 0,
		nb_sell = Number(input_nb_sell.value) || 0,
		buy_price = Number(input_buy_price.value) || 0,
		sell_price = Number(input_sell_price.value) || 0,
		fees = Number(input_fees_percent.value) || 0,

		fees_buy_nb = r7(nb_buy * fees/100),
		fees_buy_price = r4(fees_buy_nb * buy_price);


	var	total_buy_price = r4(nb_buy * buy_price),
		total_sell_price = r4(nb_sell * sell_price),

		fees_sell_price = r4(total_sell_price*fees/100),

		total_fees = fees_buy_price + fees_sell_price,
		gain = r2(((total_sell_price - total_buy_price) - fees_sell_price)),
		roi = total_buy_price!=0 ? r2(gain*100/total_buy_price) : 0;

	label_fb.label = "-" + fees_buy_nb + " BTC (" + fees_buy_price + " "+cc+")";
	label_fs.label = "-" + fees_sell_price + " "+cc;
	label_total_buy.label = "INVEST: "+total_buy_price+" "+cc;
	label_total_sell.label = "TOTAL: "+total_sell_price+" "+cc;
	label_gain.label = "RETURN: "+gain+" "+cc;
	label_roi.label = "ROI: "+roi+" %";
	label_roi.background = "#000000";
	label_roi.color = roi>0 ? "#00BB00" : "#AA0000";
};

var setCurrency = function(c){
	c = isNaN(c) ? 0 : c;
	c = c.bound(0, currencies.length);
	currentDevise = currencies[c];
	ddCurrency.value = c;
	label_CURR.label = currentDevise;
	updateSecretFormula();
};

/* MTGOX TICKER ------------------------------------------------------------- */

var setCurrentBTCPrice = function(json){
	if (json.result != "success") return false;

	var last_price = r3(json.data.last.value),
		buy_price = r3(json.data.buy.value),
		sell_price = r3(json.data.sell.value);

	input_sell_price.value = last_price;
	updateSecretFormula();
};

var getCurrentBTCPrice = function(){
	document.status.label = "Loading BTC Price ...";
	document.status.open();

	var url = "http://data.mtgox.com/api/2/BTC"+currentDevise+"/money/ticker_fast";

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

getLocal();
getCurrentBTCPrice();
