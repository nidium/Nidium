/* ------------------------+------------- */
/* Native Framework 2.0    | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 Stight.com - Vincent Fontaine */
/* -------------------------------------- */

var main = new Application({
	background : "rgba(38, 39, 34, 1)",
	//backgroundImage : "falcon/assets/back.png"
});

main.status = new UIStatus(main);
main.status.open();
main.status.progressBarColor = "rgba(210, 255, 60, 1)";

//var url = "http://195.122.253.112/public/mp3/Symphony%20X/Symphony%20X%20'A%20Fool's%20Paradise'.mp3";
var url = "http://www.desktopwallpaperhd.net/wallpapers/4/7/landscape-beach-wallpaper-background-paradise-paradisebeach-high-41319.jpg";

var h = new HttpRequest(url, function(e){
	for (var h in e.headers){
		echo(h, e.headers[h]);
	}
	main.status.label = "Complete";
	main.status.value = 0;
	main.status.close();
});

h.ondata = function(e){
	main.status.label = e.percent+"%";
	main.status.value = e.percent;
};

h.onerror = function(e){
	main.status.label = 'Error: ' + e.error;
	main.status.value = 0;
};

