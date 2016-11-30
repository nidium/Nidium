<p align="center"><img src="https://github.com/nidium/Nidium/raw/master/resources/icons/nidium_128x128.png" /><br />**nidium**<br />[![Build Status](https://travis-ci.org/nidium/Nidium.svg?branch=master)](https://travis-ci.org/nidium/Nidium)<p>

## Introduction

nidium is an ongoing effort for a general purpose rendering engine to create apps, games that run on both desktop and on mobile. Of course is it also possible to create serverside applications.

nidium helps you create graphical software with Javascript. It's **not** a NodeJS, QT, Chromium, or WebKit derivate. It has been designed from scratch and has a small codebase written in C++.

It leverages the combination of **Skia** Graphics from Google, and Mozilla's JavaScript Engine (**SpiderMonkey**) together with many more awesome librairies.

It supports various common well known API such as :

* **WebGL**
* **Canvas 2D Context**
* **WebSocket** (client & server)
* **Module loading** (require())

And some other non standard :

* **UDP/TCP Socket** (client & server)
* **HTTP** (Client & server)
* **File access**
* **Threading**
* **Threaded Audio**
* **Videos**
* **Fragment shader on 2D canvas**
* **Local Key/Value database**

It can seamlessly run various Javascript libraries like Three.JS, PixiJS, Phaser (and probably a lot more without much modification).

## Layout engine

nidium ships with its own layout engine. That is, every element has its own memory buffer (retained mode), pretty similar to HTML elements.

The layout engine allows several "layout" operations to be made on each element (relative position, opacity, drag'n'drop, margin, scrolling, overflow, and so on).

It also implements a simple HTML DOM compatility layer and is mature enough to run a MVVM framework like [Vue.js](https://github.com/vuejs/vue) and probably [React](https://github.com/facebook/react) with little to no effort.

## Network and event loop

nidium uses its own high-performance library ([libapenetwork](https://github.com/nidium/libapenetwork)) which handles all the networking operations in a non-blocking, async way.

## Download nidium

You can download nidium binaries for Linux and OSX from the [download page](http://www.nidium.com/downloads/).

## Building nidium

To build nidium you need at least 5GB of disk space. A build from scratch could take 30 to 90 minutes, depending of the speed of your computer.

On a debian based system, a few commands will get you started.

```
$ apt-get install libpci-dev python2.7 git make patch clang pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev yasm libasound2 libasound2-dev libbz2-1.0
$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```

## Documentation

We strive to have an exellent documentation, both for our [getting started guid](http://www.nidium.com/docs/guide/get-started/hello-world.html) and for the [API Reference](http://www.nidium.com/docs/api/).

## Bug reports and Collaboration

Feel free to report any bug or issue [to us](https://github.com/nidium/Nidium/issues).

If you feel that something is not clear, that could be a issue worth reporting and solving.

## License

Copyright 2016 Nidium Inc. All rights reserved.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.
