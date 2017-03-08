Compliation of Nidium on windows
================================

There are some dependencies needed to get nidium compiling on windows.
Here are some steps, we are documentating things as we move along.

We will be compiling ONLY with MSVC on the windows platform.
We won't use clang, cygwin or mingw! 
We have put a lot of efford to create some 'configure_*' scripts that are using the command line tools of visualstudio, not the gui.
You can use the gui for your convieniance, but you'll get the same error messages and warnings when you compile it in a terminal.

We are focussing on windows 64 bit OS, but 32 bit might work with a little finetuning.

(BTW: That this port was started on the branch with the name 'windows-x86' does not have any significant meaning.

# 64x Windows Host

Do you have a windows machine? Probably not, but you can test one for 90 days.

* Download and install virtualbox
* Download and install one of the 64 bit images from https://developer.microsoft.com/en-us/microsoft-edge/tools/vms/
* Unpack the download, you might want to copy and rename it
* Start the virtual box
* Import the appliance, and configure it as you like.
  (e.g. 4 processors, 4G, 10 mb video for fullscreen, 25G disk)
* Start it up
* Do your thing: disable defragmentation for ssd, set the clock, antivirus, firewall, windows updates, users&passwords, networkconfig, keyboard settings, ..

# Rights

The 'konstruktor.py' build tools does a lot of symlink juggling. By default, (ntfs?) needs more rights.

* Start 'secpol.msc' as administrator
* 'Local Policies' > 'User Rights Assignement' > 'Create symbolic links'
   add your user to the groups...
* Start 'gpupdate /force'
* Log in as your user

# Software

## git

Obviously,

* Download from https://git-scm.com/download/win
* Install, add git to your path and also add the binutils (pr.exe) for your convienienc.
* You might want to set your information

```
$ git config --global user.name "John Doe"
$ git config --global user.email johndoe@example.com
```

And to keep our commit history a little cleaner, we would appreciate something like this:

```
git config --global pull.rebase true
git config --global core.autocrlf input
```

ymmv, but there is kdiff3 in the mozilla-build directory

```
git config --global merge.tool kdiff3
git config --global mergetool.kdiff3.path "c:/mozilla-build/kdiff3/kdiff3.exe"
git config --global --add mergetool.kdiff3.trustExitCode false

git config --global --add diff.guitool kdiff3
git config --global --add difftool.kdiff3.path "c:/mozilla-build/kdiff3/kdiff3.exe"
git config --global --add difftool.kdiff3.trustExitCode false
```

## Microsoft Visual Studio Community edition 2015

* Download from http://www.microsoft.com/en-us/download/details.aspx?id=48146
* Run
  - choose custom;
  - select 'Visual Studio 2015 Update 3'
  - select 'Programming Languages|Visual C++|Common Tools for Visual C++ 2015
  - select 'Visual C++
  - unselect select 'Windows and Web Development|Universal Windows App Development Tools|Windows 10 SDK (10.0.10586)"
  - unselect 'Microsoft Web Developer Tools'
  - next, the installation takes longer then a coffe break, so you could continue with the steps below.

When the installation is done, you can take another coffeebreak to start Visual Studio for the first time, but it is not needed: Our intention is that you can hack on nidium with just your favorite editor and the configure_* scripts.

## Mozilla build

To build spidermonkey we well reuse parts of the mozilla buildprocess, this has a lot of unixy tools, including bash, sed, awk, sort, grep, find, vim, which etc

* Download from https://wiki.mozilla.org/MozillaBuild
* Run and install it in 'c:\mozilla-build'. I don't like to place stuff in the root, but it appears to be an essential place.
  Please do not get annoyed about the stuff msys2 stuff that will be installed in c:\mozilla-build\msys although git did provide that as well.
* Standard this comes with python 2.7.11 Which has a problem with urllib and https. And konstructor.py needs that.
* Move the directory c:\mozilla-build\python to c:\mozilla-build\python-2.7.11 (or remove it)

## svn

Angle will start depot_tools to download svn.
For some reason that fails, let's install it manually.

* Download from https://storage.googleapis.com/chrome-infra/svn_bin.zip.
* Install and put it in your path

```
cd c:\Data\ProgramFiles\
c:\mozilla-build\7zip\7z.exe x c:\Data\Downloads\svn_bin.zip
set PATH=%PATH;C:\Data\ProgramFiles\svn_bin
```

# python

The konstruktor.py script is written in python.

* Download from https://www.python.org/downloads/release/python-2713/
  you can pick any version as long as it is higher the 2.7.11 and lower then 2.8 
* Install this in 'c:\mozilla-build\python'
* It is wise to update pip

```
c:\mozilla-build\python\python -m pip install --upgrade pip
```

* Install the stuff that was originally in the mozilla-build/python-2.7.11

```
c:\mozilla-build\python\python.exe -m pip install hgwatchman
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

# cmake

* Download from https://cmake.org/download/
* Install

## Finetune your bash experience

~/.bashrc

```
export PYTHONPATH=/c/Data/nidium/NidiumTools/src
export VIRTUALENV_PYTHON=/c/mozilla-build/python/python.exe
export PATH=$PATH:/c/Program\ files/Git/bin:/c/Data/ProgramFiles/svn_bin:/c/Program\ Files/Cmake/bin:/c/Data/ProgramFiles/binutils/bin:
cd /c/Data/nidium/Nidium
```

~.bash_profile

```
# generated by Git for Windows
test -f ~/.profile && . ~/.profile
test -f ~/.bashrc && . ~/.bashrc
```

## Optional, but productivity is king

### watchman

Facebook's watchman is not as intuitive as 'entrproject.org', but it is allready included in mozilla-build. So we can reuse it with something like:

```
/c/mozilla-build/watchman/watchman.exe watch src gyp patch
/c/mozilla-build/watchman/watchman.exe -- trigger compile_nidium src gyp patch -- ./configure_libnidiumcore
```

### console

In most cases cmd.exe will suffice, copy and paste that works, there is mintty.exe in both "c:\mozilla-builds\msys\mintty.exe" and c:\Program Files\Git\mingw32\bin
If you want multiple tabs etc checkout https://www.fosshub.com/ConEmu.html.

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

If you are a vim user but want to work inside Visual Studio's gui, you can install vsvim by clicking:
Tools - Extensions and Updates - online - Visual studio Gallery -  CTRL+E - vsvim - Download - Install - Restart now.

### clover 

Tab browsing in file explorer

* Download from http://ejie.me/download/
* Install, guessing the buttons is a bit scary

# Checkout of nidium

## Cloning nidium from github

### Basic checkout

```
mkdir c:\Data\nidium
cd c:\Data\nidium
git clone https://github.com/nidium/NidiumTools.git
git clone --recursive https://github.com/nidium/Nidium.git

```
## Set up the paths

Mozilla has a nice script to prepare the environment with all the mvsc bells and wistles.

```
c:\mozilla-build\start-shell-msvc2015-x64.bat
```

A 'bash' shell opens and you are in your 'home' directory.

## Create some missing symlinks

```
ln -s /c/mozilla-build/python/python.exe /usr/bin/python2.7
```

# Compiling of nidium

## For 'now'

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
./configure_server
```

At this point there will be +-12 complilation errors, when they are resolved it is time for some testing.

```
./configure_server --unit-tests --auto-tests --asume-yes
```

## For 'in the near future'

```
./configure_frontend --ignore-build=basekit,libcoroutine,skia,angle
./configure_frontend --unit-tests --auto-tests --asume-yes
```

At this point there will be .. complilation errors,

# For 'in the far future'

```
./configure_frontend
```

## For 'In the far future, in a galaxy far away'

```
cd Nidium
./configure_frontend
```

# Running nidium

```
./bin/nidium
```

# Hack nidium

* Fire up your favorite editor or msvc with ./gyp/all.sln.
* Take the time to read our getting started for developers guide.
* Pick one of our simple issues or let your imagination go.
* Have fun

# Make nidium apps

* fire up your favorite editor
* Take the time to read the "getting started guide and browse through the api documentation
* Create an nml file and let your imagination go.
* Have fun