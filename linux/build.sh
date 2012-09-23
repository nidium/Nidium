#!/bin/sh
g++ \
    main.cpp \
    -I ../SDL/include/ \
    -I ../src/ \
    -I ../network/ \
    -I ../mozilla/js/src/dist/include/ \
    -L ../SDL/build/build/.libs/ \
    -L ../skia/out/Release/obj.target/gyp/ \
    -L ../mozilla/js/src/ \
    -L ../src/ \
    -L ../network/ \
    -L ../c-ares/.libs/ \
    ../SDL/build/build/.libs/libSDL2.a -lGL -lfreetype -lrt \
    -Wl,--start-group \
    -lnativestudio -lnativenetwork -lcares -lsfnt -ljs_static -lzlib -ljpeg -lopts_ssse3 -lopts -lutils -lpicture_utils -lports -limages -lskgr -lgr -leffects -lcore -Wl,--end-group
