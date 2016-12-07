#!/usr/bin/env bash

#   Copyright 2016 Nidium Inc. All rights reserved.
#   Use of this source code is governed by a MIT license
#   that can be found in the LICENSE file.

rm -fr nidium.iconset/
mkdir nidium.iconset/

printf "=> Creating iconset\n"
cp ../icons/nidium_16x16.png nidium.iconset/icon_16x16.png
cp ../icons/nidium_32x32.png nidium.iconset/icon_16x16@2x.png

cp ../icons/nidium_32x32.png nidium.iconset/icon_32x32.png
cp ../icons/nidium_64x64.png nidium.iconset/icon_32x32@2x.png

cp ../icons/nidium_128x128.png nidium.iconset/icon_128x128.png
cp ../icons/nidium_256x256.png nidium.iconset/icon_128x128@2x.png

cp ../icons/nidium_256x256.png nidium.iconset/icon_256x256.png
cp ../icons/nidium_512x512.png nidium.iconset/icon_256x256@2x.png

cp ../icons/nidium_512x512.png nidium.iconset/icon_512x512.png
cp ../icons/nidium_1024x1024.png nidium.iconset/icon_512x512@2x.png

printf "=> Generating icns\n"
iconutil -c icns nidium.iconset

printf "=> Done !\n"
