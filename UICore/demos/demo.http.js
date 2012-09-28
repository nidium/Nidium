/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

var h = new HTTP("http://code.downthemall.net/maierman/metalink-test/test.png");
h.get(function(e){
	if (e.type == "image") {
		canvas.drawImage(e.data, 0, 0);
	}
});

var file = new DataStream("http://www.google.fr/logo.gif");
file.buffer(4096);
file.chunk(2048);
file.seek(154555);
file.ondata = function(data){
	for (var i=0; i<data.length; i++){
		echo(data[i]);
	}
};
file.open();
file.oncomplete = function(data){};
file.onclose = function(){};
