<p align="center"><img src="https://github.com/nidium/Nidium/raw/master/resources/icons/nidium.iconset/nidium_128x128.png" /><br /><strong>nidium</strong><br /><a href="https://travis-ci.org/nidium/Nidium"><img src="https://travis-ci.org/nidium/Nidium.svg?branch=master" /></a><p>

## Introduction

nidium is an ongoing effort for a general purpose rendering engine to create apps and games that run on both desktop and mobile. It also offers a way to create server-side applications through **nidium-server** which shares all the non graphics related code base.

nidium helps you create graphical software with Javascript. It's **not** a NodeJS, QT, Chromium, or WebKit derivative. It has been designed from scratch and has a small codebase written in C++.

It leverages the combination of **Skia** Graphics from Google, and Mozilla's JavaScript Engine (**SpiderMonkey**) together with many more awesome libraries.

It supports various common and well known APIs such as:

* **WebGL**
* **Canvas 2D Context**
* **WebSocket** (client & server)
* **Module loading** (require())

And some other non standard:

* **UDP/TCP Socket** (client & server)
* **HTTP** (client & server)
* **File access**
* **Threading**
* **Threaded Audio**
* **Videos**
* **Fragment shader on 2D canvas**
* **Local Key/Value database**

It can seamlessly run various JavaScript libraries like Three.JS, PixiJS, Phaser (and probably a lot more without much modification).

## What problem(s) is nidium trying to solve

To put it simply, nidium aims to expose something close to web technology but in an easily hackable environment.

We're not trying to copy the Web. Think of it as a sandbox where you can quickly prototype new things, without any dependency to blink/webkit (e.g. like Electron does). This allows us to add various optimizations and non standard things. Moreover, nidium is small (nidium statically linked with all of its dependencies is about 20 MB).

Also, nidium aims to target a large spectrum of devices and our short term goal is to run it on low-power devices (mobile, rpi, ...)

By the way, it uses SpiderMonkey as its JavaScript engine, and we believe that Mozilla needs some love too!

## Work in progress

nidium is still a **work in progress** and several key features are missing:

* App distribution (create actual app out of nidium)
* Microsoft Windows support
* Good documentation

## Layout engine

nidium ships with its own layout engine. That is, every element has its own memory buffer (retained mode), pretty similar to HTML elements.

The layout engine allows several "layout" operations to be made on each element (relative position, opacity, drag'n'drop, margin, scrolling, overflow, and so on).

It also implements a simple HTML DOM compatility layer and is mature enough to run a MVVM framework like [Vue.js](https://github.com/vuejs/vue) and probably [React](https://github.com/facebook/react) with little to no effort.

## Network and event loop

nidium uses its own high-performance library ([libapenetwork](https://github.com/nidium/libapenetwork)) which handles all the networking operations in a non-blocking, async way.

## Download nidium

You can download nidium binaries for Linux and OSX from the [download page](http://www.nidium.com/downloads/).

## Building nidium

To build nidium you need at least 5.7GB of disk space. A build from scratch may take 30 to 90 minutes, depending of the speed of your computer.

On a debian based system, a few commands will get you started.

```
$ apt-get install libpci-dev python2.7 git make patch clang pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev libosmesa6-dev yasm libasound2 libasound2-dev libbz2-1.0
$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```

On a mac-os based system you need xcode installed.

```
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```

## Documentation

We strive to have excellent documentation, both for our [getting started guide](http://www.nidium.com/docs/guide/get-started/hello-world.html) and for the [API Reference](http://www.nidium.com/docs/api/).

## Videos - nidium in action

- [http://p.nf/nidium-vids/](http://p.nf/nidium-vids/)

## Bug reports and Collaboration

Feel free to report any bug or issue [to us](https://github.com/nidium/Nidium/issues).

If you feel that something is not clear, that could be an issue worth reporting and solving.

## License

Copyright 2017 Nidium Inc. All rights reserved.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.
