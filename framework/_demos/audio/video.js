var c = new Canvas(1024, 768);
Native.canvas.add(c);
//c.ctx.imageSmoothingEnabled = true;
//c.ctx.scale(1.5, 1.5);

var a = Audio.getContext(1024, 2, 44100);
var target = a.createNode("target", 2, 0);

var v = new Video(c);
v.onplay = function() {
    console.log("playing");
}
v.onbuffered = function() {
    console.log("Buffered " + v.width + " / " + v.height);
    
}
v.onerror = function(err, errStr) {
    console.log("NativeViode error " + err + "/" + errStr);
}
var va;
//File.read("/mnt/stockage/dev/android/android/frameworks/base/media/tests/contents/media_api/videoeditor/MPEG4_SP_640x480_15fps_1200kbps_AACLC_48khz_64kbps_m_1_17.mp4", function(data, size) {
//File.read("/mnt/stockage/dev/android/android/cts/tests/res/raw/video_480x360_mp4_h264_500kbps_30fps_aac_stereo_128kbps_44100hz.mp4", function(data, size) {
//File.read("/mnt/stockage/public/chuck/Chuck.S04E15.HDTV.XviD-LOL.avi", function(data, size) {
//File.read("/tmp/foo-1.mp4", function(data, size) {
//File.read("/mnt/stockage/tmp/foo.mp4", function(data, size) {
//File.read("/tmp/foobar.mov", function(data, size) {
//File.read("/home/efyx/projet/meelyaV2/rapport_1/cd/site/movies/ace.flv", function(data, size) {
//File.read("/home/efyx/.titanium/appcelerator-titanium_mobile-878906d/demos/KitchenSink/Resources/movie.mp4", function(data, size) {
//File.read("/tmp/song.mp3", function(data, size) {
    //v.setSize(624, 352);
v.open("/home/efyx/dizzy.mp4");
v.onready = function() {

    va = v.getAudioNode();
    if (va != null) {
        echo("Audio is not null");
        a.connect(va.output(0), target.input(0));
        a.connect(va.output(1), target.input(1));
    } 
    v.play();
    console.log(v.duration);
    console.log(JSON.stringify(v.metadata));
    setInterval(function() {
        console.log("Position is at " + v.position);
    }, 500);
}
v.onframe = function() {
    console.log("on frame");
}
console.log(v.canvas);

var paused = false;
window._onkeydown = function(ev) { 
    switch (ev.keyCode) {
        case 1073741903:
            var pos = v.position;
            v.position = (pos + 60);
            console.log("Setting position to " + (pos+60));
            //v.play();
        break;
        case 1073741904:
            var pos = v.position;
            v.position = (pos- 60);
            //v.play();
            console.log("Setting position to " + (pos-60));
        break;
        case 32:
            if (paused) {
                v.play();
            } else {
                v.pause();
            }
            paused = !paused;
            console.log("Setting play/pause");
        break;
    }
};
