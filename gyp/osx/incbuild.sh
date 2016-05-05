#!/bin/bash

# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

bN=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "$1/Info.plist")
bN=$(($bN + 1))
bN=$(printf "%d" $bN)
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $bN" "$1/Info.plist"

