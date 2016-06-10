#!/usr/bin/env bash

platform=`uname`
if [[ $platform == 'Linux' ]]; then
    inkscape=inkscape
    function resize {
        convert $1 -resize $2 $3
    }
elif [[ $platform == 'Darwin' ]]; then
    inkscape=/Applications/Inkscape.app/Contents/Resources/bin/inkscape
    function resize {
        sips -z $2 $2 $1 --out $3
    }
fi

printf "=> Converting svg to png\n"
$inkscape $(pwd)/nidium.svg --export-png $(pwd)/nidium.png --export-dpi 300

printf "=> Generating icon variants\n"

resize nidium.png 16    nidium_16x16.png
resize nidium.png 32    nidium_32x32.png
resize nidium.png 64    nidium_64x64.png
resize nidium.png 128   nidium_128x128.png
resize nidium.png 256   nidium_256x256.png
resize nidium.png 512   nidium_512x512.png
resize nidium.png 1024  nidium_1024x1024.png

if [[ $platform == 'Darwin' ]]; then
    cd ../osx/
    ./update_icns.sh
    cd -
fi

printf "=> Done !\n"
