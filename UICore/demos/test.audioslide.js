var main = new Application();

var HTTPAudioRequest = function(url, cb) {
    var r  = new Http(url).request(function(e) {
        if (e.type == "audio") {
            cb(e.data);
        } else {
            echo("no data");
        }
    });
}

var slider3 = main.add("UISliderController", {
    x : 350,
    y : 240,
    w : 250,
    h : 20,

    displayLabel : true,
    labelBackground : 'rgba(255, 255, 255, 0.7)',
    labelColor : 'rgba(0, 0, 0, 0.5)',
    labelOffset : -18,
    labelWidth : 56,
    labelPrefix : 'f=',
    labelSuffix : ' %',
    fontSize : 10,
    lineHeight : 18,

    splitColor : 'rgba(0, 0, 0, 0.5)',
    boxColor : 'rgba(255, 255, 255, 0.02)',

    disabled : false,
    radius : 2,
    min : 0,
    max : 5,
    value : 0.5
});

HTTPAudioRequest("http://f.z.nf/song.mp3", function(data) {
    source.open(data);
    source.play();

});

/*
HTTPAudioRequest("http://labs.swelen.com/games/ztype4coders/media/music/endure.ogg", function(data) {
    source2.open(data);
    source2.play();
});
*/


var dsp = new Audio(1024, 2, 44100);

var source = dsp.createNode("source", 0, 2);
var source2 = dsp.createNode("source", 0, 2);
var gain = dsp.createNode("gain", 2, 2);
var target = dsp.createNode("target", 2, 0);

dsp.connect(source.output(0), gain.input(0));
//dsp.connect(source.output(1), gain.input(1));

//dsp.connect(source2.output(0), gain.input(0));
//dsp.connect(source2.output(1), gain.input(1));

dsp.connect(gain.output(0), target.input(0));
dsp.connect(gain.output(1), target.input(1));

slider3.addEventListener("change", function(value){
    gain.set("gain", value);
}, false);
