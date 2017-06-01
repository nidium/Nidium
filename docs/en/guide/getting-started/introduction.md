---
description: What is nidium
---

> The rendering engine for modern mobile applications

## What is nidium?

nidium is a complete ecosystem for building modern mobile applications using web technologies.  
In practice, this means that you write your mobile application using JavaScript and markup language.

Unlike other commonly used framework, nidium is not based on a webview or any existing rendering engine.  
Behind the scene, it implements **its own rendering engine** (somewhat comparable to what a Browser does).

Related topic : [Reinventing the weel]

## What problems is nidium trying to solve?

nidium aims to expose an easily hackable environment for app developers. We designed the whole thing to be as close as possible to app development using web technologies.

With nidium, you can experiment without being dependent to blink or webkit, like Electron is for example. Moreover, nidium’s footprint is small (about 20 Mb statically linked with all dependencies and it typically consumes just a few megabytes of virtual memory).

We strive to target a large spectrum of devices, and our short term goal is to ship on **major mobile operating systems**.

[Reinventing the weel]: {{ Guide.URL('misc', 'reinventing-the-wheel') }}

## Developer experience and ease of use

nidium was designed with **developer experience** (DX) in mind.  
Unlike existing solutions, nidium doesn't require extensive use of the command line or third-party like Babel or Webpack.

### Debugging

nidium supports **Chrome DevTools** out of the box.

### Emulator

It comes with its own mobile emulator which usually boot in less than **1 second**.
