#!/bin/sh
export CXX="/usr/bin/clang++"
export GYP_DEFINES="clang=1"
GYP=../third-party/gyp/gyp
$GYP -d all --include=config.gypi --include=common.gypi --depth ./ all.gyp
$GYP -d all --include=config.gypi --include=../modules/common.gypi --depth ../modules/ ../modules/all.gyp
