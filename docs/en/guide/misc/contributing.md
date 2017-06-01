---
description: Contributing to nidium
---

[warning: **nidium** is currently under heavy development and we are looking for contributors to join our efforts. The main goal of nidium is to enable developers to build cross platform application with JavaScript.]

Depending of your skills you can help us on various areas : 
- If you know **C/C++** by porting nidium to a new platform, fixing bugs or, adding new features.
- If you know **JavaScript** help us to improve the [HTML5 compatibility layer](https://github.com/nidium/Nidium/blob/master/src/Embed/html5.js) for supporting more well-known JavaScript libraries. You can also build awesome applications or library unleashing the power of nidium.
- If you know **Python** you can contribute to our build & documentation system
-  Review & improve our [documentation](https://github.com/nidium/Nidium/tree/master/docs)
- Test nidium on your computer/devices and report bugs.
- Spread the word about nidium : social network, blog, etc.

If you are interested in contributing to nidium, drop us a message on [Slack](http://nidium.slack.com), [IRC](irc://irc.freenode.net/nidium) or [GitHub](https://github.com/nidium/Nidium/issues) so we can help you and give you insights.

## Overview of nidium projects

- **nidium** : Our main product, also known as **nidium Frontend** enable you to build graphical software with JavaScript. It expose various API including Canvas, WebGL, Audio, Timers, Sockets, Threading, Files, ...
- **nidium server**: A version of nidium tailored for servers (no graphical interface/API)
- **libnidiumcore**: A library for spidermonkey embedders with all the core features of nidium (Timers, Sockets, Threading, Files)
- **libapenetwork**: A fast cross-platform async network library. This library is used as the main event loop of nidium.
- **Konstructor**: A python script that handle the building of third-party librairies and other usefull stuff.
- **Dokumentor**: A python script that generate our documentation.

## Building nidium

To build nidium you need about 6GB of disk space. A build from scratch may take 30 min to a few hours, depending of the speed of your computer.

On a debian based system, a few commands will get you started.

```bash
$ apt-get install libpci-dev python2.7 git make patch clang pkg-config libgtk2.0-dev libgtk-3-dev mesa-common-dev libglu1-mesa-dev libosmesa6-dev yasm libasound2 libasound2-dev libbz2-1.0
$ git clone https://github.com/nidium/NidiumTools.git
$ git clone --recursive https://github.com/nidium/Nidium.git
$ export PYTHONPATH=$(pwd)/NidiumTools/src
$ cd Nidium
$ ./configure_frontend
$ ./bin/nidium
```
Three configure script are available for building the product of your choice :
 
- `configure_frontend`
- `configure_server`
- `configure_libnidiumcore` 

Each of these configure script accept various options. The most useful one are : 

- `--debug` : Build nidium (and spidermonkey) in debug mode, when developping on nidium, you should always enable this flag as it will help you to catch errors sooner. 
> Note : debug build result in increased binary size and reduced performance.

- `--asan` : Enable clang address sanitizer and leak sanitizer. 
- `--cpu-profiling` : Enable CPU profiling trough `gperftools`.
- `--unit-tests` : Build & Run unit-tests.
- `--help` : Show all options & usage.

## nidium directory structure
<table>
    <tr>
        <td>Directory</td>
        <td>Usage</td>
    </tr>
    <tr>
          <td><code>bin/</code></td>
         <td>nidium executable</td>
    </tr>
    <tr>
          <td><code>build/</code></td>
         <td>Build files for nidium and third party</td>
    </tr>
    <tr>
          <td><code>docs/</code></td>
         <td>Documentation of JavaScript bindings using doukmentor</td>
    </tr>
    <tr>
          <td><code>gyp/</code></td>
         <td>Build files for nidium. We are using <a href="https://gyp.gsrc.io/">GYP</a> as a tool to generate the buildfiles for each platform.</td>
    </tr>
    <tr>
          <td><code>patch/</code></td>
         <td>Patch for  third party libraries.</td>
    </tr>
    <tr>
          <td><code>resources/</code></td>
         <td>Icons, OS specific files resources.</td>
    </tr>
<tr>
        <td><code>src</code></td>
        <td><b>Nidium source code</b></td>
    </tr>
    <tr>
          <td><code>src/AV/</code></td>
         <td>Audio & Video framework</td>
    </tr>
    <tr>
          <td><code>src/Binding/</code></td>
         <td>JavaScript Bindings</td>
    </tr>
    <tr>
          <td><code>src/Core/</code></td>
         <td>Various base helper (Path, Args, DB, SharedMessages, TaskManager, Context)</td>
    </tr>
    <tr>
          <td><code>src/Embed/</code></td>
         <td>JavaScript files bundled with nidium. Contains various additions to native JS class.</td>
    </tr>
    <tr>
          <td><code>src/Frontend/</code></td>
         <td>Code related to nidium frontend (NML, Context, Application entrypoint for each OS)</td>
    </tr>
    <tr>
          <td><code>src/Graphics/</code></td>
         <td>OpenGL related code (Canvas 2D & 3D, Image, Skia interactions)</td>
    </tr>
     <tr>
          <td><code>src/IO/</code></td>
         <td>Input/Output related code (File, Stream)</td>
    </tr>
    <tr>
          <td><code>src/Interface/</code></td>
         <td>Specific implementation for each OS : 
         <ul>
             <li>System :  System directories, Notifications, Language.</li>
             <li>Interface : Window creation/interaction, Menu, File picker, Input hander</li>
             </ul>
        </td>
    </tr>
    <tr>
          <td><code>src/Net/</code></td>
         <td>Implementation of HTTP and WebSocket protocol (client & server) </td>
    </tr>
    <tr>
          <td><code>src/Server/</code></td>
         <td>Nidium server implementation</td>
    </tr>
    <tr>
          <td><code>src/libapenetwork/</code></td>
         <td>libapenetwork subrepo (event loop)</td>
    </tr>
    <tr>
          <td><code>tests/</code></td>
         <td><b>Nidium unit-tests</b></td>
    </tr>
    <tr>
          <td><code>tests/gunittest/</code></td>
         <td>Low-level tests in C++, based on the <a href="https://github.com/google/googletest">googletest</a> framework.</td>
    </tr>
    <tr>
          <td><code>tests/jsunittests/</code></td>
         <td>JavaScript unit-tests for nidium bindings.</td>
    </tr>
    <tr>
          <td><code>tests/jsautotest/</code></td>
         <td>JavaScript unit-tests automatically created by the examples from the documentation.</td>
    </tr>
     <tr>
          <td><code>tests/tests-server/</code></td>
         <td>HTTP server used for network tests (Socket, HTTP, WebSocket). 
         One pubic server is running at http://tests.nidium.com but you can run your own. Check the <a href="https://github.com/nidium/Nidium/blob/master/tests/tests-server/README.md">README.md</a> file in the directory.</td>
    </tr>
     <tr>
          <td><code>third-party/</code></td>
         <td>Third-party libraries downloaded by the <code>configure</code> script when they are needed in the build-process. <br/><br/>
<em>Note : The extracted directories are symlinked to this directory depending of the configuration used during the <code>configure</code> script. The real files are stored in hidden directories (<code>.default</code> <code>.default-debug</code>, ...</em>)
        </td>
     <tr>
          <td><code>tools/</code></td>
         <td>Nidium Installer for each platform & various tools</td>
    </tr>
</table>

## Coding practices

- Nidium use a subset of `C++11`, general usage of the standard library is allowed but :
    - Don't use C++ stream.
- In C++ code, use `nullptr` for pointers. In C code, use `NULL`.
- Forward-declare classes in your header files instead of including them whenever possible.
- Don't put an else right after a return. Delete the else, it's unnecessary and increases indentation level.
- For logging use [nidium logging system](https://github.com/nidium/Nidium/wiki/Logging).
- For JavaScript binding use [ClassMapper](https://github.com/nidium/Nidium/wiki/ClassMapper).
- Rooting of JavaScript object/value can be tricky, make sure to read our [rooting guide](https://github.com/nidium/Nidium/wiki/JSAPI-Rooting-Howto).
- If you are adding a new features, make sure to add unit-tests and documentation to your code.

## Coding Style

In general, follow the style of the surrounding code, and adhere to the following style guide.
Code will be read often, so structured, and readable code is welcome, as well as sensible naming of variables, classes, functions etc.

- No tabs. No whitespace at the end of a line.
- Indentation is 4 spaces
- Unix-style linebreaks (`\n`), not Windows-style (`\r\n`).
- 80 characters or less
- Use K&R style for control structure, class and function declaration.
- Pointer star go with the variable name not the type `Foo *foo = new Foo();`
- Use camelCase for variables, methods and functions :
    - `int myFunction();`
    - `ClasName::methodName();`
    - `int myVar = 0;`
- Use CamelCase for class class, member variable, static method, namespace
    - `ClassName::StaticMethod`
    - `namespace Nidium`
- Use uppercase for macros or define (e.g `MY_MACRO`). You can use lowercase for function like macro (e.g `ndm_log()`)
- Prefixes : 
    - `g_`=global `g_Global`
    - `m_`=member (variable) `m_VariableName`
    - `k` Used for constant inside enum e.g. `enum HTTPMethod { kHTTPMethod_get }`
- Namespace : Nidium use one namespace per directory in `src/`
    - `using` statements are not allowed in header files (We don't want to pollute the global scope of compilation units that use the header file.)
    - Don't indent code inside namespace

**Control structure style : **
```
if (...) {
} else if (...) {
} else {
}

// Short conditional statements may be written on one line if this enhances readability.
if (...) x = z;

// Always use curly brace for single-line statements
if (...) {
    x = z;
}

while (...) {
}

for (...; ...; ...) {
}

switch (...) {
  case 1: {
    // When you need to declare a variable in a switch, put the block in braces
    int var;
    break;
  }
  case 2:
    ...
    break;
  default:
    break;
}
```
**Class, Namespace, Function:**
```
namespace Nidium {

class MyClass : public A
{
public:
    MyClass(int var, int anotherVar)
        : m_Var(var), m_Var2(anotherVar)
    {
       ...
    }

    // Tiny constructors and destructors can be written on a single line.
    MyClass() { }

    // Tiny functions can be written in a single line.
    int myFunction() { return mVar; }

    static int StaticMethod()
    {
        ...
    }
    
private:
    // Member variable are prefixed with m_
    int m_Var;
    int m_Var2 = 0; // Variable should be initialised when declared
};

} // namespace Nidium
```
**License block :**

Nidium MIT license should be but at the top of each new file.

For C/C++/JavaScript :
```
/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
```

For Python and Bash : 
```
# Copyright 2017 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.
```
