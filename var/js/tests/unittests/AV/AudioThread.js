Tests.registerAsysnc("Audio.run/onmessage", function() {
    Audio.run(function() { 
        this.send("Hello");
    });
    Audio.onmessage = function() {
    }
}, 5000);
