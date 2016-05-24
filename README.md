# Nidium - Tool-kit for writing server, games, desktop software and mobile apps

## Background

In the 1990's it was impossible to picture what the web would become by the year 2014.

Nidium is the first attempt to handle what's coming: the fusion of desktop applications, mobile applications and next-generation webapps.

It is a brand new solution to create and browse next-generation web and server based applications that run on every platform.

Nidium is so simple to use that you can create apps in minutes. Edit your javascript, add your stylesheet and distribute it.

hello.nml

```
<application version="0.2">
    <meta>
        <title>My Application</title>
        <description>Such a great description</description>
        <viewport>355x140</viewport>
        <identifier>com.company.app</identifier>
        <author>Company Inc.</author>
    </meta>
    <assets>
        <style src="style.nss"></style>
        <script src="hello.js"></script>
    </assets>
    </application>
```

style.nss

```
{
    "button" : {
        background : "blue",
        color : "rgba(255, 255, 255, 0.8)"
        label : "Default"
    },

    "label" : {
        background : "black",
        color : "#ffffff"
    }
}
```
hello.js

```
var b = new UIButton(document);
b.label = "Hello World";
b.center();
```

## License

Copyright 2016 Nidium Inc. All rights reserved.
Use of this source code is governed by a MIT license that can be found in the LICENSE file.


## How to get up and running

You can start by doing something like this :

```
$ apt-get install git subversion \
                  wget \
                  make cmake \
                  python2.7 gcc g++ clang3.5 yasm/nasm \
                  libpng12-dev libgif-dev libpng-dev mesa-common-dev libglu1-mesa-dev libfreetype6-dev libgtk2.0-dev \
                  libasound2 libbz2-1.0 libjack0
$ git clone --recursive git@github.com:nidium/NativeStudio.git
$ cd NativeStudio
$ ./configure
```
