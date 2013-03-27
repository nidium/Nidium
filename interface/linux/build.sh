#!/bin/sh

    #-lportaudio -lzita-resampler
g++ \
    main.cpp \
    -fno-rtti -ffunction-sections -fdata-sections -fno-exceptions -DNDEBUG -DTRIMMED -g -O3 -freorder-blocks  -fomit-frame-pointer \
    -I ../../third-party/SDL/include/ \
    -I ../ \
    -I ../../src/ \
    -I ../../network/ \
    -I ../../third-party/mozilla/js/src/dist/include/ \
    -L ../ \
    -L ../../third-party/SDL/build/.libs/ \
    -L ../../third-party/skia//out/Release/obj.target/gyp/ \
    -L ../../third-party/mozilla/js/src/ \
    -L ../../src/ \
    -L ../../network/ \
    -L ../../audio/ \
    -L ../../http-parser/ \
    -L ../../third-party/ffmpeg/libavformat/ \
    -L ../../third-party/ffmpeg/libavcodec/ \
    -L ../../third-party/ffmpeg/libavutil/ \
    -L ../../third-party/ffmpeg/libswscale/ \
    -L ../../third-party/ffmpeg/libswresample/ \
    -L ../../c-ares/.libs/ \
    -L ../../third-party/angle/out/Debug/obj.target/src/ \
    -Wl,--start-group \
    ../../third-party/SDL/build/build/.libs/libSDL2.a -lGL -lfreetype -lrt -lz -lpthread -lpng \
    ../../third-party/libzip/lib/.libs/libzip.a  \
    ../../third-party/jsoncpp/libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.a \
    ../../third-party/portaudio/lib/.libs/libportaudio.a \
    ../../third-party/zita-resampler/libs/libzita-resampler.a \
    /usr/lib/i386-linux-gnu/libasound.so.2 \
    -ldl \
    -lnativeaudio \
    -lhttp_parser \
    -lnativeinterface \
    -lswresample -lswscale -lavformat -lavcodec -lavutil  \
    -ltranslator_glsl -ltranslator_common -lpreprocessor \
    -lnativestudio -lnativenetwork -lcares -lskia_sfnt -lzlib -ljpeg -lskia_opts_ssse3 -lskia_opts -lskia_utils -lpicture_utils -lskia_ports -lskia_images -lskia_skgr -lskia_gr -lskia_effects -lskia_core \
    -ljs_static -lnspr4 \
    -Wl,--end-group

mv a.out ../../framework/
