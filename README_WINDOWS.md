Compliation of Nidium on windows
================================

There are rather a lot dependencies and tweaks needed.
Here are some steps, documentating things as we move along.

# Do you have a windows machine? Probably not, but you can test one for 90 days.

* download and install virtualbox
* download and install one of the images from https://developer.microsoft.com/en-us/microsoft-edge/tools/vms/
* unpack the download, you might want to copy and rename it
* start the virtual box
* import the appliance, and configure it as you which (e.g. 4 processors, 4G,
  20G extra disk)
* start it
* do your thing: windows updates, users&passwords, networkconfig, ..
* log in as your user

# konstruktor.py and rights

The konstruktor.py build tools does a lot of symlink juggling. By default, (ntfs?) needs more rights.

* start 'secpol.msc' in a cmd prompt as an administrator
* 'Local Policies' > 'User Rights Assignement' > 'Create symbolic links'
   add your user to the groups...
* 'gpupdate /force'

# Microsoft visual studio community edition 2015

We will be compiling ONLY with MSVC on the windows platform, so let's get that
first. We won't use clang, cygwin or mingw on this platform.

* download from http://www.microsoft.com/en-us/download/details.aspx?id=48146
* run
  - choose custom;
  - select 'Visual Studio 2015 Update 3'
  - select 'Programming Languages|Visual C++|Common Tools for Visual C++ 2015
  - select 'Visual C++
  - unselect 'Microsoft Web Developer Tools'
  next, .... and time for a coffee break
* start that stuff for the first time...., may take a second coffee break.

# Mozilla build

To build spidermonkey we well reuse parts of the mozilla buildprocess, this has a lot of unixy tools, including bash, sed, awk, sort, grep, find, vim, which etc

* download from https://wiki.mozilla.org/MozillaBuild
* run and install it in 'c:\mozilla-build'. I don't like to place stuff in the root, but it appears to be an essential place.
* Standard this comes with python 2.7.11 Which has a problem with urllib and https. And konstructor.py needs that.
* move the directory c:\mozilla-build\python to c:\mozilla-build\python-2.7.11 (or remove it)

# python

The konstruktor.py script is written in python.

* download from https://www.python.org/downloads/release/python-2713/
  you can pick any version as long as it is higher the 2.7.11 and lower then 2.8 
* install this in 'c:\mozilla-build\python'
* it is wise to update pip
  c:\mozilla-build\python\python -m pip install --upgrade-pip
* install the stuff that was in the mozilla-build/python-2.7.11

```
c:\mozilla-build\python\python.exe -m pip install virtualenv hgwatchman mercurial
```
'hgwatchman' gives an error, that we will ignore now; we don'tr realy need
hgwatchman and mercurial any way.
We might non need virtualbox, as mozilla-central will provide an older version as well. 

# pywin32

The konstruktor.py script does a lot of filesystem operations, pywin32 makes that possible.
 
* Download from https://sourceforge.net/projects/pywin32/files/pywin32/, sorry about that...
* Install and choose c:\mozilla-build\python 

# git

Obviously

* download from https://git-scm.com/download/win
  Please do not get annoyed about the stuff msys2 stuff that will be installed
although that was already in c:\mozilla-build\msys

# Optional, but productivity is king

## console

In most cases cmd.exe will suffice, copy and paste that works, there is mintty.exe in both "c:\mozilla-builds\msys\mintty.exe" and c:\Program Files\Git\mingw32\bin
If you want multiple tabs etc checkout https://www.fosshub.com/ConEmu.html or https://sourceforge.net/projects/console/

```
mkdir c:\Data\ProgramFiles\ConEmuPack
cd c:\Data\ProgramFiles\ConEmuPack
c:\mozilla-build\7zip\7z.exe x c:\Data\Downloads\ConEmuPack.161206.7z
```

## Tiling window managers

ymmv on this, but since we have a git and visual studio, there are only a few
steps needed:

```
cd c:\data\
git clone https://github.com/ZaneA/HashTWM.git
"c:\Program Files\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"
MSBuild.exe HashTWM\vsproj\HashTWM.sln /p:PlatformToolset=v140
```

start it with `c:\Data\HashTWM\bin\hashtwm.exe`

## gvim

ymmv obviously, but there are emacs and vim' s in c:\mozilla-build\

* download from http://www.vim.org/download.php/#pc

# Checkout of nidium

## cloning nidium from github

### Basic checkout

```
mkdir c:\Data\nidium
cd c:\Data\nidium
git checkout https://github.com/nidium/NidiumTools.git
git clone https://github.com/nidium/NidiumTools.git
git clone --recursive https://github.com/nidium/Nidium.git

```

## set up the path for the mozilla and the msvc stuff.

```
c:\mozilla-build\start-shell-msvc2015.bat
#or 
c:\mozilla-build\start-shell-msvc2015-x64.bat
```

A 'bash' shell opens and you are in your 'home' directory.

```
cd /c/Data/nidium
export PYTHONPATH=`pwd`/NidiumTools/src
export VIRTUALENV_PYTHON=/c/mozilla-build/python/python.exe
export PATH=$PATH:/c/Program\ Files/Git/bin
ln -s /c/mozilla-build/python/python.exe /usr/bin/python2.7
export PATH=$PATH:/c/Program\ files/Git/bin
```

### For 'now'

While we are still working on the windows port, things are a bit
different...

```
cd NidiumTools
set PYTHONPATH=%cd%\src
git fetch origin windows-x86
git checkout windows-x86
cd ..\Nidium
git fetch origin windows-x86
git checkout windows-x86
cd src\libapenetwork
git fetch origin windows-x86
git checkout windows-x86
cd ..\..
python configure_libnidiumcore --ignore-build=all

## For 'in the near future'
#python configure_server --ignore-build=all
#python configure_frontend --ignore-build=all

## For 'in the far future'
#python configure_libnidiumcore --unit-tests --auto-tests --asume-yes
#python configure_server --unit-tests --auto-tests --asume-yes
#python configure_frontend --unit-tests --auto-tests --asume-yes
```

### For 'In the far future, in a galaxy far away'

```
cd Nidium
python configure_frontend
```

# Run it

```
./bin/nidium
```


