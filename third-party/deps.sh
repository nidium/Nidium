#!/bin/bash -e

#Process the arguments
for i in "$@"
do
case $i in
    --os=*)
        OS=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
    ;;
    --autoconf=*)
        AUTOCONF=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
    ;;
    --output=*)
        OUTPUT=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
    ;;
    --build=*)
        BUILD=`echo $i | sed 's/[-a-zA-Z0-9]*=//'`
    ;;
    *)
        #huh?
    ;;
esac
done

LIBS_DIR=$OUTPUT/third-party-libs/.libs/
OUTPUT_DIR=$OUTPUT/third-party-libs/$BUILD/

mkdir -p $OUTPUT/third-party-libs/.libs/
mkdir -p $OUTPUT/third-party-libs/release/
mkdir -p $OUTPUT/third-party-libs/debug/

if [[ "$OS" == "linux" ]]; then
    LINUX=1
else
    OSX=1
fi

if [[ $LINUX ]]; then
    NBCPU=`nproc`
else
    NBCPU=`sysctl -n hw.ncpu`
fi

AUTOCONF=/usr/bin/autoconf2.13

# Download everything

if [ -d "mozilla-central" ]; then
    echo "mozilla-central already downloaded"
else 
    echo "Downloading mozilla central..."
    curl http://hg.mozilla.org/mozilla-central/archive/tip.tar.bz2 | tar xj
    mv `ls |grep mozilla-*` mozilla-central
fi

if [ -d "SDL2" ]; then
    echo "SDL2 already downloaded"
else 
    echo "Downloading SDL2 ... "
    curl http://www.libsdl.org/tmp/SDL-2.0.tar.gz | tar zx
    mv `ls |grep SDL-2*` SDL2
fi

if [ -d "skia" ]; then
    echo "Skia already downloaded..."
else
    echo "Downloading skia..."
    svn checkout http://skia.googlecode.com/svn/trunk skia
fi

if [ -d "jsoncpp" ]; then
    echo "jsoncpp already downloaded"
else
    echo "Downloading jsoncpp... "
    curl "http://switch.dl.sourceforge.net/project/jsoncpp/jsoncpp/0.5.0/jsoncpp-src-0.5.0.tar.gz" |tar zx
    mv `ls |grep jsoncpp-src*` jsoncpp
fi

if [ -d "c-ares" ]; then
    echo "c-ares already downloaded"
else
    echo "Download c-ares..."
    curl http://c-ares.haxx.se/download/c-ares-1.9.1.tar.gz | tar zx && mv c-ares-1.9.1 c-ares
fi

if [ -d "libzip" ]; then
    echo "libzip already downloaded"
else
    echo "Downloading libzip..."
    curl http://www.nih.at/libzip/libzip-0.10.1.tar.bz2 | tar xj && mv libzip-0.10.1 libzip
fi

if [ -d "ffmpeg" ]; then
    echo "ffmpeg already downloaded"
else
    echo "Downloading ffmpeg..."
    curl http://www.ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2 | tar xj 
fi

if [ -d "portaudio" ]; then
    echo "portaudio already downloaded"
else
    echo "Downloading portaudio..."
    curl http://www.portaudio.com/archives/pa_stable_v19_20111121.tgz |tar zx
fi

if [ -d "zita-resampler" ]; then
    echo "zita-resampler already downloaded"
else
    echo "Downloading zita-resampler..."
    curl http://kokkinizita.linuxaudio.org/linuxaudio/downloads/zita-resampler-1.3.0.tar.bz2 |tar xj && mv zita-resampler-1.3.0 zita-resampler
    echo "Patching zita-resampler Makefile..."
    patch -p0 < zita.patch
fi

#Build phase
echo "Everything download. Lets build!" 

if [ -e $OUTPUT_DIR/libjs_static.a ]; then
    echo "Mozilla central already build"
else
    echo "Building nspr from mozilla central..."
    cd mozilla-central/nsprpub/
    ./configure && make -j$NBCPU
    NSPRDIR=`pwd`
    cd ../js/src/
    echo "Building spidermonkey from mozilla central..."
    $AUTOCONF
    if [[ $BUILD == "debug" ]]; then
        ./configure --enable-debug --disable-optimize --enable-valgrind --enable-ctypes --with-nspr-cflags="-I$NSPRDIR/dist/include/nspr/" --with-nspr-libs="$NSPRDIR/dist/lib/libnspr4.a $NSPRDIR/dist/lib/libplds4.a $NSPRDIR/dist/lib/libplc4.a"
    else 
        ./configure --enable-threadsafe --enable-ctypes --with-nspr-cflags="-I$NSPRDIR/dist/include/nspr/" --with-nspr-libs="$NSPRDIR/dist/lib/libnspr4.a $NSPRDIR/dist/lib/libplds4.a $NSPRDIR/dist/lib/libplc4.a"
    fi
    make -j$NBCPU
    cd ../../../
fi

if [ -e $LIBS_DIR/libSDL2.a ]; then
    echo "SDL2 already build"
else
    cd SDL2/
    mkdir -p build/ && cd build/
    ../configure && make -j$NBCPU
    cd ../../
fi

if [ -e $LIBS_DIR/libskia_core.a ]; then
    echo "Skia already build"
else
    echo "Building skia..."
    cd skia 
    export GYP_DEFINES='skia_arch_width=32'
    ./gyp_skia
    make tests BUILDTYPE=Release -j$NBCPU
fi

if [ -e $LIBS_DIR/libjson_linux-gcc-4.6_libmt.a ]; then
    echo "jsoncpp already build"
else
    cd jsoncpp/
    echo "Downloading scons... "
    curl "http://freefr.dl.sourceforge.net/project/scons/scons-local/2.2.0/scons-local-2.2.0.tar.gz" | tar zx
    echo "Running scons and building jsoncpp..."
    python scons.py platform=linux-gcc
    cd ../
fi

if [ -e $LIBS_DIR/libcares.a ]; then
    echo "c-ares already build"
else
    echo "Building c-ares..."
    cd c-ares && ./configure && make -j$NBCPU
    cd ../
fi

if [ -e $LIBS_DIR/libzip.a ]; then
    echo "libzip already build"
else 
    echo "Building libzip...."
    cd libzip && ./configure && make -j$NBCPU
    cd ../
fi

if [ -e $LIBS_DIR/libhttp_parser.a ]; then
    echo "libhttp_parser already build"
else
    echo "Building libhttp_parser...."
    cd http-parser
    make
    make package
    cd ../
fi

if [ -e $LIBS_DIR/libavcodec.a ]; then
    echo "FFmpeg already build"
else
echo "Building ffmpeg...."
    cd ffmpeg 
    ./configure \
        --disable-avdevice --disable-postproc --disable-avfilter --disable-programs --disable-ffplay --disable-ffserver --disable-ffprobe --disable-everything \
        --enable-decoder=mp3,vorbis,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8 \
        --enable-parser=vorbis,mpegaudio \
        --enable-demuxer=mp3,ogg,vorbis,pcm_alaw,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_mulaw,pcm_s16be,pcm_s16le,pcm_s24be,pcm_s24le,pcm_s32be,pcm_s32le,pcm_s8,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8 \
    make -j$NBCPU
    cd ../
fi

if [ -e $LIBS_DIR/libportaudio.a ]; then
    echo "Portaudio already build"
else
    echo "Building portaudio..."
    cd portaudio && ./configure && make -j$NBCPU
    cd ../
fi

if [ -e $LIBS_DIR/libzita-resampler.a ]; then
    echo "zita-resampler already build"
else
    echo "Building zita-resampler..."
    cd zita-resampler/libs/ && make -j$NBCPU
    cd ../
fi

echo "Copying library to output dir"
cp ./ffmpeg/libavcodec/libavcodec.a $LIBS_DIR 
cp ./ffmpeg/libswresample/libswresample.a $LIBS_DIR 
cp ./ffmpeg/libavutil/libavutil.a $LIBS_DIR 
cp ./ffmpeg/libavformat/libavformat.a $LIBS_DIR 
cp ./ffmpeg/libswscale/libswscale.a $LIBS_DIR 
cp ./skia/out/Release/libskia_pdf.a $LIBS_DIR 
cp ./skia/out/Release/libskia_ports.a $LIBS_DIR 
cp ./skia/out/Release/libskia_skgr.a $LIBS_DIR 
cp ./skia/out/Release/libskia_gr.a $LIBS_DIR 
cp ./skia/out/Release/libcityhash.a $LIBS_DIR 
cp ./skia/out/Release/libskia_utils.a $LIBS_DIR 
cp ./skia/out/Release/libskia_effects.a $LIBS_DIR 
cp ./skia/out/Release/libskia_core.a $LIBS_DIR 
cp ./skia/out/Release/libskia_sfnt.a $LIBS_DIR 
cp ./skia/out/Release/libskia_images.a $LIBS_DIR 
cp ./skia/out/Release/libskia_opts_ssse3.a $LIBS_DIR 
cp ./skia/out/Release/libskia_opts.a $LIBS_DIR 
cp ./SDL2/build/build/.libs/libSDL2.a $LIBS_DIR 
cp ./c-ares/.libs/libcares.a $LIBS_DIR 
cp ./zita-resampler/libs/libzita-resampler.a $LIBS_DIR 
cp ./libzip/lib/.libs/libzip.a $LIBS_DIR 
cp ./portaudio/lib/.libs/libportaudio.a $LIBS_DIR 
#cp ./mozilla-central/js/src/ctypes/libffi/.libs/libffi.a $LIBS_DIR 
#cp ./mozilla-central/js/src/ctypes/libffi/.libs/libffi_convenience.a $LIBS_DIR 
cp ./mozilla-central/nsprpub/dist/lib/libnspr4.a $LIBS_DIR 
cp ./jsoncpp/libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.a $LIBS_DIR 
cp ./http-parser/libhttp_parser.a $LIBS_DIR
cp ./mozilla-central/js/src/libjs_static.a $OUTPUT_DIR

ln -s $LIBS_DIR/libavcodec.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libswresample.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libavutil.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libavformat.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libswscale.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_pdf.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_ports.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_skgr.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_gr.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libcityhash.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_utils.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_effects.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_core.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_sfnt.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_images.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_opts_ssse3.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libskia_opts.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libSDL2.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libcares.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libzita-resampler.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libzip.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libportaudio.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libnspr4.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libhttp_parser.a $OUTPUT_DIR 
ln -s $LIBS_DIR/libjson_linux-gcc-4.6_libmt.a $OUTPUT_DIR 

echo "All done!" 
