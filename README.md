[![Build Status](https://travis-ci.org/nidium/Nidium.svg?branch=master)](https://travis-ci.org/nidium/Nidium)

<p align="center"><img src="https://github.com/nidium/Nidium/raw/master/resources/icons/nidium_128x128.png" /><br />**nidium**<p>

nidium is an ongoing effort for a general purpose rendering engine to create apps, games on both Desktop and mobile and also serverside applications.

nidium helps you create graphical softwares with JavaScript. It's **not** NodeJS, QT, Chromium, or a WebKit derivate. It has been designed from scratch and has a small codebase written in C++.

It leverages the combination of **Skia** Graphics from Google, and Mozilla's JavaScript Engine (**SpiderMonkey**) and many more awesome librairies. 

It supports various common well known API such as :

* **WebGL**
* **Canvas 2D Context**
* **WebSocket** (client & server)
* **Module loading** (require())

And some other non standard :

* **UDP/TCP Socket**
* **Files**
* **Threaded Audio**
* **videos** (libav integration)
* **Fragment shader on 2d canvas**
* **HTTP** (Client & server)
* **Local storage**

It can seemlessly runs various library like Three.JS, PixiJS, Phaser and probably a lot more without much modification.

## Layout engine

nidium also ships with its own layout engine. That is, every element has its own memory buffer (retained mode), pretty similar to HTML elements.

The layout engine allows several "layout" operations to be made on each element (relative position, opacity, drag'n'drop, margin, scrolling, overflow, and so on).  

It also implements a simple HTML DOM compatility layer and is recently able to run various MVVM framework like [Vue.js](https://github.com/vuejs/vue) and probably [React](https://github.com/facebook/react) with little to no effort.

## Network and event loop

nidium use its own library ([libapenetwork](https://github.com/nidium/libapenetwork)) which handle all the networking operations.


## Building nidium

To build nidium you need at least 5GB of disk space. A build from scratch will take between 30min to 1H30, depending of the speed of your computer.

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
