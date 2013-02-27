var c = new Canvas(1024, 768);
Native.canvas.add(c);
c.ctx.imageSmoothingEnabled = true;
c.ctx.scale(1.5, 1.5);

var a = new Audio(1024, 2, 44100);
var target = a.createNode("target", 2, 0);

var v = new Video(a, c);
v.onplay = function() {
    echo("PLAYING-----------------------------------------");
}
v.onbuffered = function() {
    echo("PLAYING-----------------------------------------" + v.width + " / " + v.height);
    
}
v.onerror = function(err) {
    console.log("NativeViode error " + err);
}
var va;
//File.read("/mnt/stockage/dev/android/android/frameworks/base/media/tests/contents/media_api/videoeditor/MPEG4_SP_640x480_15fps_1200kbps_AACLC_48khz_64kbps_m_1_17.mp4", function(data, size) {
//File.read("/mnt/stockage/dev/android/android/cts/tests/res/raw/video_480x360_mp4_h264_500kbps_30fps_aac_stereo_128kbps_44100hz.mp4", function(data, size) {
//File.read("/mnt/stockage/public/chuck/Chuck.S04E15.HDTV.XviD-LOL.avi", function(data, size) {
File.read("/tmp/song.mp3", function(data, size) {
//File.read("/mnt/stockage/tmp/foo.mp4", function(data, size) {
//File.read("/mnt/stockage/tmp/new.avi", function(data, size) {
//File.read("/home/efyx/projet/meelyaV2/rapport_1/cd/site/movies/ace.flv", function(data, size) {
//File.read("/home/efyx/.titanium/appcelerator-titanium_mobile-878906d/demos/KitchenSink/Resources/movie.mp4", function(data, size) {
//File.read("/tmp/song.mp3", function(data, size) {
    //v.setSize(624, 352);
    echo("Opening");
    v.open(data);
    echo("Opened");

    va = v.getAudioNode();
    if (va != null) {
        echo("Audio is not null");
        a.connect(va.output(0), target.input(0));
        a.connect(va.output(1), target.input(1));
    } else {
        echo("Null audio :(");
    }
    echo("Playing");
    v.play();
    echo("Play sent");
});
