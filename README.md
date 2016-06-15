# ɳidium

ɳidium enables you to build graphical softwares with JavaScript. It's not NodeJS, QT, Chromium, or a WebKit derivate, It's built from scratch, has a small codebase written in c++, and leverage the combination of Skia Graphics toolkit from Google and SpiderMonkey JavaScript engine from Mozilla. 

## Download

You can find Nidium latest build on http://downloads.nidium.com/

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

## Building Nidium

To build Nidium you need at least 5GB of disk space. Depending of the speed of your computer a build from scratch will take between 30min to 1H30.

```
$ apt-get install python2.7 git build-essential clang cmake pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev yasm libasound2 libasound2-dev libbz2-1.0

$ git clone https@github.com/nidium/NidiumTools.git
$ git clone --recursive https@github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```

## License

Copyright 2016 Nidium Inc. All rights reserved.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.

## FAQ

**Q:** _What is the prupose of Nidium?_

**A:** Building graphical application with JavaScript and without the browser stack.

---

**Q:** _Does Nidium require a GPU?_

**A:** Yes

---

**Q:** _What platform Nidium currently support?_

**A:** As of today Nidium runs on `linux` and `OSX`. The port to Android is a work in progress. Window port is in our top priorty list. 
Contact us if you would like us to increase focus on a specific platform.

---

**Q:** _Are you aware that Nidium is a kind of game-engine?_

**A:** Yes & No. It does an amazing job at rendering but there is no physics-engine or 3D engine built in.

---

**Q:** _Are you aware that Nidium is a kind of browser?_

**A:** Yes & No. Nidium has similarities with a browser, but it's definitely not a browser (No HTML)

---

**Q:** _How does it compare to `node-webkit` or `electron` ?_

**A:**  Nidium is not built on top of a browser.

---

**Q:** _How does it compare to `flash`?_

**A:** Nidium just like flash use vector graphics, but Nidium is not a plugin that run in the browser, it use JavaScript and not ActionScript. And of course, it's open source :)

---

**Q:** _How does it compare to `openframeworks`?_

**A:** There you program in c++; Nidium lets you use plain javascript.

---

**Q:** _Why did you make a server version?_

**A:** The core components were designed to be reused as a library.
   We needed a very fast, low latency, robust server. As the async network layer is very fast and we dogfooded Nidium to outperform Node.js/Socket.IO. 
   This server has been tested in production environement (>810 M req / day).
   Don't forget that Nidium (frontend) has the same server built in, which can give your application amazing posibilities.

---

**Q:** _Will there be a kind of `comet` or `push` module?_

**A:** Yes! This project is build around the networklayer that was also used in the
   [APE-Project](http://ape-project.org/). You can expect some cool things in a near future.


**Q:** _Can I use `ES6` features?_

**A:** Yes [several](https://kangax.github.io/compat-table/es6/#firefox31). Currently 
   we are focussing to add more features and modules; but we will defenitly 
   extend es6 support.

---

**Q:** _Can i reuse `Node.js` modules?_

**A:** Yes & No. Nidium has support for `require()` but NodeJS API has not been ported to Nidium. So this means that any modules that use NodeJS specific API/features will not work. 

---

**Q:** _Any plans to make Nidium compatible with NodeJS API?_

**A:** Yes, but this is not _yet_ on our top priority list. 

---

**Q:** _What is this `konstrucktor.py`?_

**A:** That is a nifty tool to download dependencies, generate gyp files, and pass 
   configuration flags around.

---

**Q:** _Can I persuade you to focus on `...` ?_

**A:** Sure! Head over our slack channel and chat with us :)

---

**Q:** _Can I help?_

**A:** Sure!
