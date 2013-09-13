/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

document.status.open();

var url = "http://195.122.253.112/public/mp3/Symphony%20X/Symphony%20X%20'A%20Fool's%20Paradise'.mp3";
//var url = "http://www.desktopwallpaperhd.net/wallpapers/4/7/landscape-beach-wallpaper-background-paradise-paradisebeach-high-41319.jpg";

var h = new HttpRequest('GET', url, null, function(e){
	for (var h in e.headers){
		console.log(h, e.headers[h]);
	}
	document.status.label = "Complete";
	document.status.value = 0;
	document.status.close();
});

h.ondata = function(e){
	document.status.label = e.percent+"%";
	document.status.value = e.percent;
};

h.onerror = function(e){
	document.status.label = 'Error: ' + e.error;
	document.status.value = 0;
};

