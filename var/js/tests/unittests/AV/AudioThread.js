// This test case is a bit flawed as we are not checking
// that the code runs inside the audio thread. 
// However it will at least check for any crash
Tests.registerAsync("Audio.run", function(next) {
    var dsp = Audio.getContext();

    dsp.run(function() { 
        console.log("hello world");
    });

    setTimeout(function() {
        next();
    }, 1000);

}, 5000);

