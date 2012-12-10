##!/bin/sh
#g++ \
#    main.cpp \
#    -fno-rtti -ffunction-sections -fdata-sections -fno-exceptions -DNDEBUG -DTRIMMED -g -O3 -freorder-blocks  -fomit-frame-pointer \
#    -I ../../SDL/SDL2/include/ \
#    -I ../src/ \
#    -I ../network/ \
#    -I ../mozilla/js/src/dist/include/ \
#    -L ../SDL/build/build/.libs/ \
#    -L ../skia/out/Release/obj.target/gyp/ \
#    -L ../mozilla/js/src/ \
#    -L ../src/ \
#    -L ../network/ \
#    -L ../c-ares/.libs/ \
#    ../../SDL/build/.libs/libSDL2.a -lGL -lfreetype -lrt -lz -lpthread -lpng \
#    -Wl,--start-group \
#    -lnativestudio -lnativenetwork -lcares -lsfnt -ljs_static -lzlib -ljpeg -lopts_ssse3 -lopts -lutils -lpicture_utils -lports -limages -lskgr -lgr -leffects -lcore -Wl,--end-group
#
#cp a.out ../UICore/

#!/bin/sh
g++ \
    main.cpp \
    -fno-rtti -ffunction-sections -fdata-sections -fno-exceptions -DTRIMMED -g -O3 -freorder-blocks  -fomit-frame-pointer \
    -I ../SDL/include/ \
    -I ../src/ \
    -I ../network/ \
    -I ../audio/ \
    -I /home/efyx/dev/mozilla-central/js/src/dist/include/ \
    -L /home/efyx/dev/mozilla-central/js/src/ \
    -L ../SDL/build/build/.libs/ \
    -L ../skia-read-only/out/Release/obj.target/gyp/ \
    -L ../../../mozilla-central/js/src/ \
    -L ../src/ \
    -L ../network/ \
    -L ../audio/ \
    -L ../../ffmpeg/libavformat/ \
    -L ../../ffmpeg/libavcodec/ \
    -L ../../ffmpeg/libavutil/ \
    -L ../c-ares/.libs/ \
    ../SDL/build/build/.libs/libSDL2.a -lGL -lfreetype -lrt -lz -lpthread -lpng \
    -Wl,--start-group \
    -lnspr4 \
    -ljs_static \
    -lnativeaudio \
    -lavformat -lavcodec -lavutil -lportaudio -lzita-resampler \
    -lnativestudio -lnativenetwork -lcares -lskia_sfnt -lzlib -ljpeg -lskia_opts_ssse3 -lskia_opts -lskia_utils -lpicture_utils -lskia_ports -lskia_images -lskia_skgr -lskia_gr -lskia_effects -lskia_core \
    -Wl,--end-group

cp a.out ../UICore/
