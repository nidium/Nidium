[![Build Status](https://travis-ci.org/nidium/Nidium.svg?branch=classmapper)](https://travis-ci.org/nidium/Nidium)

# ɳidium

ɳidium helps you create graphical softwares with JavaScript. It's not NodeJS, QT, Chromium, or a WebKit derivate. It has been designed from scratch and has a small codebase written in C++. ɳidium leverages the combination of Skia Graphics from Google, and Mozilla's JavaScript Engine (SpiderMonkey). 

## Download

You can find ɳidium's latest builds on http://downloads.nidium.com/

## Hello world

hello.nml

```
<application>
    <meta>
        <title>My Application</title>
        <description>Such a great description</description>
        <viewport>300x300</viewport>
        <identifier>com.company.app</identifier>
        <author>Company Inc.</author>
    </meta>
    <assets>
        <script>
            var c = new Canvas(200, 200);
            var ctx = c.getContext("2d");

            document.canvas.add(c);

            ctx.fillStyle = "blue";
            ctx.fillRect(0, 0, 200, 200);
        </script>
    </assets>
    </application>
```

## Building ɳidium

To build ɳidium you need at least 5GB of disk space. A build from scratch will take between 30min to 1H30, depending of the speed of your computer.

```
$ apt-get install libpci-dev python2.7 git make patch clang pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev yasm libasound2 libasound2-dev libbz2-1.0

$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```

## License

Copyright 2016 Nidium Inc. All rights reserved.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.

## FAQ

**Q:** _What is the prupose of ɳidium?_

**A:** Creating high performance JavaScript applications without the browser stack.

---

**Q:** _Does ɳidium require a GPU?_

**A:** Yes

---

**Q:** _What platform ɳidium currently support?_

**A:** As of today, ɳidium runs on `linux` and `OSX`. The port to Android is still a work in progress. Windows port is in our top priority list.  
Feel free to contact us if you need information on a specific platform.

---

**Q:** _Can I use ɳidium as a kind of game-engine?_

**A:** Yes. It does an amazing job at rendering, but there is no physic-engine behind, or any 3D engine built in.

---

**Q:** _Is ɳidium a kind of browser?_

**A:** Yes & No. Nidium has similarities with a browser, but it's definitely not a browser (No HTML)

---

**Q:** _How does it compare to `node-webkit` or `electron` ?_

**A:**  ɳidium is not built on top of a browser.

---

**Q:** _How does it compare to `flash`?_

**A:** Both ɳidium and flash use vector graphics, but ɳidium is not a browser plugin. With ɳidium, you write application using JavaScript instead of ActionScript. And of course, it's open source :)

---

**Q:** _How does it compare to `openframeworks`?_

**A:** ɳidium lets you use plain JavaScript instead of C++.

---

**Q:** _Why did you make a server version?_

**A:** The core components were designed to be reused as a library.
   We needed a very fast, low latency, robust server. As the async network layer is very fast and we dogfooded ɳidium to outperform Node.js/Socket.IO. 
   This server has been tested in production environement (>810M requests / day).
   Off course, ɳidium (frontend) has the same built-in server, which can give your application amazing posibilities.

---

**Q:** _Will there be a kind of `comet` or `push` module?_

**A:** Yes! This project is build around the networklayer that was also used in the
   [APE-Project](http://ape-project.org/). You can expect some cool stuffs in a near future.

---

**Q:** _Can I use `ES6` features?_

**A:** Yes [several](https://kangax.github.io/compat-table/es6/#firefox31). Currently 
   we are focussing to add more features and modules, but we will definitely 
   extend es6 support.

---

**Q:** _Can I reuse `Node.js` modules?_

**A:** Yes & No. Nidium has support for `require()` but NodeJS API has not been ported to Nidium. So this means that any modules that use NodeJS specific API/features will not work.

---

**Q:** _Any plan to make ɳidium compatible with NodeJS API?_

**A:** Yes, but this is not _yet_ on our top priority list. 

---

**Q:** _What is this `konstrucktor.py`?_

**A:** That is a nifty tool to download dependencies, generate gyp files, and pass 
   configuration flags around.

---

**Q:** _How can I help?_

**A:** Head over our slack channel and come chat with us :)
