---
description: Installing nidium
---

[warning: Please note that we are actively working on nidium and some key features are still a work in progress:
* Microsoft Windows support
* Mobile support
* App distribution
For these reasons, nidium is **not ready** for production apps yet]

## Using pre-built binaries

Go to [http://www.nidium.com/downloads/](http://www.nidium.com/downloads/)


## Building nidium from the sources

In order to build nidium you need at least 5GB of disk space. A build from scratch could takes from 20 to 60 minutes, depending of the speed of your computer.  

  
  
### GNU/Linux dependencies
Typically, on a **debian** based system you'll require the following dependencies :

```bash
$ apt-get install libpci-dev python2.7 git make patch clang pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev yasm libasound2 libasound2-dev libbz2-1.0
```

### macOS dependencies

Building nidium on macOS require Xcode to be installed.

### Building

Building nidium on macOS or GNU/Linux :

```bash
$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```
