#!/bin/sh
export CXX=/usr/bin/clang++
export GYP_DEFINES="clang=1"
GYP=../third-party/gyp/gyp
$GYP -d all --include=config.gypi --include=common.gypi --depth ./ all.gyp
