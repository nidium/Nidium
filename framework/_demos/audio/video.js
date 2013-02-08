function add(file, half) {
    var c = new Canvas(1024, 768);
    Native.canvas.add(c);
    if (half) {
        c.top = 380;
    }
    var v = new Video(file, c);
    v.play();
}
for (let i = 0; i < 2; i++) {
    add("/tmp/foo.mp4", i%2 == 0);
}
/*
var ctx = c.getContext("2D");
ctx.fillStyle = "rgba(255,0,0,0.5)";
v.onframe = function(data, width, height) {
//    ctx.foo(data, width, height);
    ctx.fillRect(0, 0, 100, 100);
}
*/

