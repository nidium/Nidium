#!/bin/sh
export CXX="~/.vim/bin/cc_args.py /usr/bin/clang++"
export GYP_DEFINES="clang=1"
GYP=../third-party/gyp/gyp
$GYP --include=config.gypi --include=common.gypi --depth ./ all.gyp
