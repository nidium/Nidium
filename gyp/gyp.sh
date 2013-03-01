#!/bin/sh
export CXX=/usr/bin/clang++
export GYP_DEFINES="clang=1"
gyp -d all --include=config.gypi --include=common.gypi --depth ./ all.gyp
