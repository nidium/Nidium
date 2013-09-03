var h = new Http("http://p.nf/post.php").request({
	headers : {
		"User-Agent" : "firefox",
		"Content-Type" : "application/x-www-form-urlencoded",
		"Connection" : "close"
	},
	data : "gros=data&foo=bar",
	timeout : 10000
}, function(e){
	console.log(e.data);
});