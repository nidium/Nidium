/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

/*
var g = {
	val : 5,
	add : function(m){
		return m + this.val;
	},
	ctx : function(){
		this.zz = 40;
	}
};

var w = {
	wrapper : o,
	revoke : revoke
} = Membrane(g);


var h = new o.ctx();

echo(h.zz);
w.revoke();
echo(h.zz);

*/

function assertEquals(a, b){
	if (a === b) echo(a, "===", b);
}



/*
BenchThis("Native 1024x768 FillRect", 50000, function(i){
	canvas.fillRect(0, 0, 1024, 768);
});

BenchThis("Native 250x250 FillRect", 50000, function(i){
	canvas.fillRect(10, 10, 250, 250);
});
*/
