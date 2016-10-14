/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var dsp;
var source;
var customProcessor;
var gain;

Tests.register("Gain node setup", function() {
    dsp = Audio.getContext();
    source = dsp.createNode("custom-source", 0, 2);
    gain = dsp.createNode("gain", 2, 2);
    customProcessor = dsp.createNode("custom", 2, 2);
    var target = dsp.createNode("target", 2, 0);

    Assert(target instanceof AudioNodeTarget);
    Assert(gain instanceof AudioNodeGain);
    Assert(source instanceof AudioNodeSource);
    Assert(source instanceof AudioNodeCustomSource);
  
    dsp.connect(source.output[0], gain.input[0]);
    dsp.connect(source.output[1], gain.input[1]);

    dsp.connect(gain.output[0], customProcessor.input[0]);
    dsp.connect(gain.output[1], customProcessor.input[1]);

    dsp.connect(customProcessor.output[0], target.input[0]);
    dsp.connect(customProcessor.output[1], target.input[1]);
});

Tests.registerAsync("GainNode.set", function(next) {
    gain.set("gain", 0.5);

    customProcessor.addEventListener("message", function(msg) {
        customProcessor.removeEventListener("message");
        customProcessor.assignProcessor(null);
        source.assignProcessor(null);

        Assert.equal(msg.data, true);

        source.stop();

        next();
    })

    customProcessor.assignProcessor(function(frames, scope) {
        var ok = false;
        for (var i = 0; i < frames.data.length; i++) {
            if (frames.data[i][frames.size - 1] == 21) {
                ok = true;
            }
        } 
        this.send(ok);
    });

    source.assignProcessor(function(frames, scope) {
        for (var i = 0; i < frames.data.length; i++) {
            frames.data[i][frames.size - 1] = 42;
        } 
    });

    source.play();

}, 5000);
