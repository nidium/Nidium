/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

// XXX : Obviously this script will fail to run on non en-us computer...
Tests.register("Navigator.language", function() {
    Assert.equal(navigator.language, 'en');
});

