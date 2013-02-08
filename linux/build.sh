#!/bin/sh
g++ \
    main.cpp \
    -fno-rtti -ffunction-sections -fdata-sections -fno-exceptions -DNDEBUG -DTRIMMED -g -O3 -freorder-blocks  -fomit-frame-pointer \
    -I ../third-party/SDL/include/ \
    -I ../src/ \
    -I ../network/ \
    -I ../third-party/mozilla/js/src/dist/include/ \
    -L ../third-party/SDL/build/.libs/ \
    -L ../third-party/skia//out/Release/obj.target/gyp/ \
    -L ../third-party/mozilla/js/src/ \
    -L ../src/ \
    -L ../network/ \
    -L ../audio/ \
    -L ../third-party/ffmpeg/libavformat/ \
    -L ../third-party/ffmpeg/libavcodec/ \
    -L ../third-party/ffmpeg/libavutil/ \
    -L ../third-party/ffmpeg/libswscale/ \
    -L ../c-ares/.libs/ \
    -L ../third-party/angle/out/Debug/obj.target/src/ \
    ../third-party/SDL/build/build/.libs/libSDL2.a -lGL -lfreetype -lrt -lz -lpthread -lpng \
    -Wl,--start-group \
    -lnativeaudio \
    -lswscale -lavformat -lavcodec -lavutil -lportaudio -lzita-resampler \
    -ltranslator_glsl -ltranslator_common -lpreprocessor \
    -ljs_static -lnspr4 \
    -lnativestudio -lnativenetwork -lcares -lskia_sfnt -lzlib -ljpeg -lskia_opts_ssse3 -lskia_opts -lskia_utils -lpicture_utils -lskia_ports -lskia_images -lskia_skgr -lskia_gr -lskia_effects -lskia_core \
    -Wl,--end-group

cp a.out ../framework/
