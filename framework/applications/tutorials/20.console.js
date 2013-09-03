var notes = [500, 200, 100, 50, 10],
	coins = [5, 2, 1],
	cents = [10000, 5000, 2000, 1000, 500, 200, 100, 50, 20, 10, 5, 1],
	wallet = {
		notes : {},
		coins : {},
		cents : {}
	};

var amount = 350.0145619,
	value = 84.5;

console.log("Your wallet: ", amount, "BTC ("+Math.round(amount*value*100)/100+" EUR)");
console.log("")
console.log("BitCoins:")
for (var i=0; i<notes.length; i++){
	var v = notes[i],
		nb = amount/v >> 0;
	wallet.notes[i] = nb;
	amount = amount % v;
	if (nb) console.log("  - ", nb, "billets de", v, "BTC ("+nb*v*value+" EUR)");
}
for (var i=0; i<coins.length; i++){
	var v = coins[i],
		nb = amount / v >> 0;
	wallet.coins[i] = nb;
	amount = amount % v;
	if (nb) console.log("  - ", nb, "pièces de", coins[i], "BTC ("+nb*v*value+" EUR)");
}
console.log("")
console.log("BitCents:")
amount*=100000000;
for (var i=0; i<cents.length; i++){
	var v = cents[i],
		nb = amount/v >> 0;
	wallet.cents[i] = nb;
	amount = amount % v;
	if (nb) console.log("  - ", nb, "pièces de", cents[i], "cents ("+(nb*v*value/100000000)+" EUR)");
}

