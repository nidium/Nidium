#!/bin/sh

# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

export CXX="/usr/bin/clang++"
export GYP_DEFINES="clang=1"
GYP=../third-party/gyp/gyp
$GYP -d all --depth ./ all.gyp
$GYP -d all --include=config.gypi --include=../modules/common.gypi --depth ../modules/ ../modules/all.gyp

