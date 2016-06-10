#!/usr/bin/env bash

# 1) Create png from svg
#inkscape=/Applications/Inkscape.app/Contents/Resources/bin/inkscape
printf "=> Converting svg to png\n"
inkscape=inkscape
$inkscape $(pwd)/nidium.svg --export-png $(pwd)/nidium.png --export-dpi 300

# 2) Generate variant
printf "=> Generating icon variants\n"
rm -fr nidium.iconset/
mkdir nidium.iconset/

convert nidium.png -resize   16 nidium.iconset/nidium_16x16.png
convert nidium.png -resize   32 nidium.iconset/nidium_16x16@2x.png
convert nidium.png -resize   32 nidium.iconset/nidium_32x32.png
convert nidium.png -resize   64 nidium.iconset/nidium_32x32@2x.png
convert nidium.png -resize  128 nidium.iconset/nidium_128x128.png
convert nidium.png -resize  256 nidium.iconset/nidium_128x128@2x.png
convert nidium.png -resize  256 nidium.iconset/nidium_256x256.png
convert nidium.png -resize  512 nidium.iconset/nidium_256x256@2x.png
convert nidium.png -resize  512 nidium.iconset/nidium_512x512.png
convert nidium.png -resize 1024 nidium.iconset/nidium_512x512@2x.png

# 3) Create icns
#iconutil -c icns nidium.iconset
printf "=> Generating icns\n"
png2icns nidium.icns nidium.iconset/*.png
