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
                  python2.7 gcc g++ clang3.5 yasm nasm \
                  mesa-common-dev \
                  libpng12-dev libglu1-mesa-dev libfreetype6-dev libgtk2.0-dev \
                  libasound2 libbz2-1.0 libjack0
$ git clone https@github.com/nidium/NativeTools.git
$ git clone --recursive https@github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NativeTools/src
$ cd Nidium
$ ./configure_frontend
$ ./configure_server
$ ./bin/nidium
```

## FAQ

Q: Will there be a kind of push module?
A: Yes! This project is build around the networklayer that was also used in the
   [APE-Project](http://ape-project.org/).

Q: Why did you make a server version?
A: The core components were designed to be reused as a library.
   In our dayjobs we needed a very fast, low latency, robust server. 
   As the async network layer is very fast and we dogfooded nidium to outperform 
   Node.js/Socket.IO. This server is battle tested (>810 M req / day, < 60 ms)
   Don't forget that nidium (frontend) has the 'same' server built in, which can
   give your application amazing posibilities.

Q: Are you aware that nidium is a kind of gameengine?
A: Yes & No. It does an amazing job at rendering but there is no physics-engine built in.

Q: Are you aware that nidium is a kind of browser?
A: Yes & No. It does all the things that a browser does, except html.

Q: How does it compare to Node-webkit?
A: Not. That uses html; Nidium lets you use plain javascript

Q: How does it compare to electron?
A: Not. That uses html; Nidium lets you use plain javascript

Q: How does it compare to flash?
A: Not. There you program in actionscript; Nidium lets you use plain javascript

Q: How does it compare to openframeworks?
A: Not. There you program in c++; Nidium lets you use plain javascript

Q: Can I use ES6 features?
A: Yes [several](https://kangax.github.io/compat-table/es6/#firefox31). Currently 
   we are focussing to add more features and modules; but we will defenitly 
   extend es6 support.

Q: Can i reuse Node.js modules?
A: Probably yes; Nidium is designed to be API compatible with Node.js.
   No support, No guarantees, No endorsments

Q: It runs on linux, BSD, OsX. What about Windows, Android, Raspberry Py, iOS?
A: The port to the android platform is in progress, the next step will be Windows.
   Contact us if you would like us to increase focus on one or the other area.

Q: What is this konstrucktor.py?
A: That is a nifty tool to download dependencies, generate gyp files, and pass 
   configuration flags around.

Q: Can I persuade you to focus on ... ?
A: Sure!

Q: Can I help?
A: Sure!

