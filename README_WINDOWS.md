Compliation of Nidium on windows
================================

There are rather a lot dependencies and tweaks needed.
Here are some steps, we are documentating things as we move along.

# Windows Host 

Do you have a windows machine? Probably not, but you can test one for 90 days.

* download and install virtualbox
* download and install one of the images from https://developer.microsoft.com/en-us/microsoft-edge/tools/vms/
* unpack the download, you might want to copy and rename it
* start the virtual box
* import the appliance, and configure it as you like.
  (e.g. 4 processors, 4G, 20G extra disk)
* start it up
* do your thing: disable defragmentation for ssd, windows updates, users&passwords, networkconfig, keyboard settings, ..
* log in as your user

# Rights

The 'konstruktor.py' build tools does a lot of symlink juggling. By default, (ntfs?) needs more rights.

* start 'secpol.msc' as administrator
* 'Local Policies' > 'User Rights Assignement' > 'Create symbolic links'
   add your user to the groups...
* start 'gpupdate /force'

# Software

## git

Obviously,

* download from https://git-scm.com/download/win
* install, add git to your path, but not the binutils.
* you might want to set your information

```
$ git config --global user.name "John Doe"
$ git config --global user.email johndoe@example.com
```

And to keep our commit history a little cleaner, we would appreciate something like this:

```
git config --global pull.rebase true
git config --global core.autocrlf false
```

## Microsoft Visual Studio Community edition 2015

We will be compiling ONLY with MSVC on the windows platform, so let's get that
first. We won't use clang, cygwin or mingw on this platform.
We have put a lot of efford to make an environment that is independent of visualstudio.
You can use the gui, but you'll get the same error messages and warnings when you compile it in a terminal.

* download from http://www.microsoft.com/en-us/download/details.aspx?id=48146
* run
  - choose custom;
  - select 'Visual Studio 2015 Update 3'
  - select 'Programming Languages|Visual C++|Common Tools for Visual C++ 2015
  - select 'Visual C++
  - unselect select 'Windows and Web Development|Universal Windows App Development Tools|Windows 10 SDK (10.0.10586)"
  - unselect 'Microsoft Web Developer Tools'
  next, .... and time for a coffee break
* start that stuff for the first time...., may take a second coffee break.

## Mozilla build

To build spidermonkey we well reuse parts of the mozilla buildprocess, this has a lot of unixy tools, including bash, sed, awk, sort, grep, find, vim, which etc

* download from https://wiki.mozilla.org/MozillaBuild
* run and install it in 'c:\mozilla-build'. I don't like to place stuff in the root, but it appears to be an essential place.
  Please do not get annoyed about the stuff msys2 stuff that will be installed in c:\mozilla-build\msys although git did provide that as well.
* Standard this comes with python 2.7.11 Which has a problem with urllib and https. And konstructor.py needs that.
* move the directory c:\mozilla-build\python to c:\mozilla-build\python-2.7.11 (or remove it)

## ar

The configure script of giflib need 'ar' and sets arguments to 'cru', We should tell autoconf to use mvsc's lib.exe instead.
untill then we'll install 'binutils' because they are nat in the 'git' or 'mozilla-build' package.

* download from https://sourceforge.net/projects/mingw/files/MinGW/Base/binutils/binutils-2.25.1/
  Sorry, ..
* install and put them in your path
* copy libwinpthread-1.dll, libiconv-2.dll, libgcc_s_dw2-1.dll  from C:\Program Files\Git\mingw32\libexec\git-core and put them into c:\Data\ProgramFiles\binutils\bin

```
mkdir c:\Data\ProgramFiles\binutils
cd c:\Data\ProgramFiles\binutils
c:\mozilla-build\7zip\7z.exe x c:\Data\Downloads\binutils-xxx--bin.tar.xz
c:\mozilla-build\7zip\7z.exe x binutils-2.25.1-1-mingw32-bin.tar
SET path=%PATH;c:\Data\ProgramFiles\binutils\bin$
```

## pr

during the configuration of ffmpeg we need coreutils's 'pr'. It is in git\msys.
but we dont want to clutter our path with, so we copy the file.

* copy msys-intl-8.dll, msys-gcc_s-1.dll, msys-iconv-2.dll, msys-2.0.dll from C:\Program Files\Git\usr\bin and put them into c:\Data\ProgramFiles\binutils\bin

```
xcopy "c:\Program Files\Git\usr\bin\pr.exe" c:\Data\ProgramFiles\binutils\bin
```

TODO: http://sourceforge.net/projects/mingw/files/MSYS/Base/msys-core/_obsolete/coreutils-5.97-MSYS-1.0.11-2/coreutils-5.97-MSYS-1.0.11-snapshot.tar.bz2/download


cd c:\Data\ProgramFiles\binutils
## svn

angle will start depot_tools to download  svn.
For some reason that fails, let's install it manually.

* download from https://storage.googleapis.com/chrome-infra/svn_bin.zip.
* install and put it in your path

```
cd c:\Data\ProgramFiles\
c:\mozilla-build\7zip\7z.exe x c:\Data\Downloads\svn_bin.zip
SET path=%PATH;C:\Data\ProgramFiles\svn_bin
```

# python

The konstruktor.py script is written in python.

* download from https://www.python.org/downloads/release/python-2713/
  you can pick any version as long as it is higher the 2.7.11 and lower then 2.8 
* install this in 'c:\mozilla-build\python'
* it is wise to update pip

```
c:\mozilla-build\python\python -m pip install --upgrade-pip
```

* install the stuff that was originally in the mozilla-build/python-2.7.11

```
c:\mozilla-build\python\python.exe -m pip install virtualenv hgwatchman mercurial
```

We will ignore the error now; we don't need hgwatchman any way.

```
c:\mozilla-build\python\python.exe -m pip install mercurial
```

We probably don't need mercurial.

```
c:\mozilla-build\python\python.exe -m pip install virtualenv
```
We might not need virtualbox, as mozilla-central provides an older version as well. 

# pywin32

The 'konstruktor.py' script does a lot of filesystem operations, pywin32 makes that possible.
 
* Download from https://sourceforge.net/projects/pywin32/files/pywin32/, sorry about that...
* Install and choose c:\mozilla-build\python 


## Optional, but productivity is king

### watchman

Facebook's watchman is not as intuitive as 'entrproject.org', but it is allready included in mozilla-build. So we can reuse it with something like:

```
/c/mozilla-build/watchman/watchman.exe watch src gyp patch
/c/mozilla-build/watchman/watchman.exe -- trigger compile_nidium src gyp patch -- ./configure_libnidiumcore
```

### console

In most cases cmd.exe will suffice, copy and paste that works, there is mintty.exe in both "c:\mozilla-builds\msys\mintty.exe" and c:\Program Files\Git\mingw32\bin
If you want multiple tabs etc checkout https://www.fosshub.com/ConEmu.html or https://sourceforge.net/projects/console/

```
mkdir c:\Data\ProgramFiles\ConEmuPack
cd c:\Data\ProgramFiles\ConEmuPack
c:\mozilla-build\7zip\7z.exe x c:\Data\Downloads\ConEmuPack.161206.7z
```

### Tiling window managers

ymmv on this, but since we have a git and visual studio, there are only a few
steps needed:

```
cd c:\data\
git clone https://github.com/ZaneA/HashTWM.git
"c:\Program Files\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat"
MSBuild.exe HashTWM\vsproj\HashTWM.sln /p:PlatformToolset=v140
```

start it with `c:\Data\HashTWM\bin\hashtwm.exe`

### gvim

ymmv obviously, but there are emacs and vim' s in c:\mozilla-build\

* download from http://www.vim.org/download.php/#pc

### mergetool meld

There is kdiff3 in the mozilla-build directory
ymmv obviously, meld is very nice, tortoise-merge is also very good

* download from https://download.gnome.org/binaries/win32/meld/3.16/
* install it 
* register it for git

```
git config --global merge.tool meld
git config --global mergetool.meld.path "c:/Program Files/meld/bin/"
```

### clover

Tab browsing in file explorer

* download from http://ejie.me/download/
* install, guessing the buttons is a bit scary

# Checkout of nidium

## Cloning nidium from github

### Basic checkout

```
mkdir c:\Data\nidium
cd c:\Data\nidium
git checkout https://github.com/nidium/NidiumTools.git
git clone https://github.com/nidium/NidiumTools.git
git clone --recursive https://github.com/nidium/Nidium.git

```

## Set up the paths

Mozilla has a nice script to prepared the environment.
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
```
## Porting/Compiling of nidium

### For 'now'

While we are still working on the windows port, things are a bit different...

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
# At this point there will be 142? complilation errors,

## For 'in the near future'
#python configure_server --ignore-build=all
# At this point there will be 5? complilation errors,
#python configure_frontend --ignore-build=all
# At this point there will be 37? complilation errors,

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


