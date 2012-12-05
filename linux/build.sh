#!/bin/sh
g++ \
    main.cpp \
    -fno-rtti -ffunction-sections -fdata-sections -fno-exceptions -DNDEBUG -DTRIMMED -g -O3 -freorder-blocks  -fomit-frame-pointer \
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
    ../SDL/build/build/.libs/libSDL2.a -lGL -lfreetype -lrt -lz -lpthread -lpng \
    -Wl,--start-group \
    -ljs_static \
    -lnativestudio -lnativenetwork -lcares -lskia_sfnt -lzlib -ljpeg -lskia_opts_ssse3 -lskia_opts -lskia_utils -lpicture_utils -lskia_ports -lskia_images -lskia_skgr -lskia_gr -lskia_effects -lskia_core \
    -Wl,--end-group

cp a.out ../UICore/
