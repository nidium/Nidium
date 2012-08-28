#!/bin/sh
g++ \
    main.cpp \
    -I ../../SDL/SDL2/include/ \
    -I ../src/ \
    -I ../mozilla/js/src/dist/include/ \
    -L ../../SDL/build/.libs/ \
    -L ../../skia-read-only/out/Release/obj.target/gyp/ \
    -L ../mozilla/js/src/ \
    -L ../src/ \
    -lSDL2 -lGL -lfreetype\
    -Wl,--start-group \
    -lnativestudio -lsfnt -ljs_static -lzlib -ljpeg -lopts_ssse3 -lopts -lutils -lpicture_utils -lports -limages -lskgr -lgr -leffects -lcore -Wl,--end-group
