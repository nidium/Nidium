#!/bin/bash -e


# Invocation example
#./deps.sh --os=linux --autoconf=/usr/bin/autoconf2.13  --output=../out/ --build=release

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

if [[ ! $OS ]]; then
    echo "You must specify --os"
    exit
fi
if [[ ! $AUTOCONF ]]; then
    echo "You must specify --autoconf"
    exit
fi
if [[ ! $OUTPUT ]]; then
    echo "You must specify --ouput"
    exit
fi
if [[ ! $BUILD ]]; then
    echo "You must specify --build"
    exit
fi


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


DEPS_URL="http://deps.nativejs.org/"

# Download everything

if [ -d "mozilla-central" ]; then
    echo "mozilla-central already downloaded"
else 
    echo "Downloading mozilla central..."
    curl $DEPS_URL/mozilla-central.bz2 | tar xj
    mv `ls |grep mozilla-*` mozilla-central
fi

if [ -d "SDL2" ]; then
    echo "SDL2 already downloaded"
else 
    echo "Downloading SDL2 ... "
    curl $DEPS_URL/SDL-2.0.tar.gz | tar zx
    mv `ls |grep SDL-2*` SDL2
fi

if [ -d "skia" ]; then
    echo "Skia already downloaded..."
else
    echo "Downloading skia..."
    curl $DEPS_URL/skia.tar.gz | tar zx
fi

if [ -d "jsoncpp" ]; then
    echo "jsoncpp already downloaded"
else
    echo "Downloading jsoncpp... "
    curl $DEPS_URL/jsoncpp-src-0.5.0.tar.gz |tar zx
    mv `ls |grep jsoncpp-src*` jsoncpp
    echo "Downloading scons... "
    curl $DEPS_URL/2.2.0/scons-local-2.2.0.tar.gz | tar zx
fi

if [ -d "c-ares" ]; then
    echo "c-ares already downloaded"
else
    echo "Download c-ares..."
    curl $DEPS_URL/c-ares-1.9.1.tar.gz | tar zx && mv c-ares-1.9.1 c-ares
fi

if [ -d "libzip" ]; then
    echo "libzip already downloaded"
else
    echo "Downloading libzip..."
    curl $DEPS_URL/libzip-0.10.1.tar.bz2 | tar xj && mv libzip-0.10.1 libzip
fi

if [ -d "ffmpeg" ]; then
    echo "ffmpeg already downloaded"
else
    echo "Downloading ffmpeg..."
    curl $DEPS_URL/ffmpeg-snapshot.tar.bz2 | tar xj 
fi

if [ -d "portaudio" ]; then
    echo "portaudio already downloaded"
else
    echo "Downloading portaudio..."
    curl $DEPS_URL/pa_stable_v19_20111121.tgz |tar zx
fi

if [ -d "zita-resampler" ]; then
    echo "zita-resampler already downloaded"
else
    echo "Downloading zita-resampler..."
    curl $DEPS_URL/zita-resampler-1.3.0.tar.bz2 |tar xj && mv zita-resampler-1.3.0 zita-resampler
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

if [ -e $LIBS_DIR/libjsoncpp.a ]; then
    echo "jsoncpp already build"
else
    cd jsoncpp/
    echo "Running scons and building jsoncpp..."
    python scons.py platform=linux-gcc
    JSONCPP_DIR=libs/`ls -1 libs/ |grep linux-gcc`
    JSONCPP_LIB=`ls -1 $JSONCPP_DIR/libjson_linux-gcc-*_libmt.a`
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

if [ ! -e $LIBS_DIR/libavcodec.a ]; then 
    cp -v ./ffmpeg/libavcodec/libavcodec.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libswresample.a ]; then 
    cp -v ./ffmpeg/libswresample/libswresample.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libavutil.a ]; then 
    cp -v ./ffmpeg/libavutil/libavutil.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libavformat.a ]; then 
    cp -v ./ffmpeg/libavformat/libavformat.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libswscale.a ]; then 
    cp -v ./ffmpeg/libswscale/libswscale.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_pdf.a ]; then 
    cp -v ./skia/out/Release/libskia_pdf.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_ports.a ]; then 
    cp -v ./skia/out/Release/libskia_ports.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_skgr.a ]; then 
    cp -v ./skia/out/Release/libskia_skgr.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_gr.a ]; then 
    cp -v ./skia/out/Release/libskia_gr.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libcityhash.a ]; then 
    cp -v ./skia/out/Release/libcityhash.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_utils.a ]; then 
    cp -v ./skia/out/Release/libskia_utils.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_effects.a ]; then 
    cp -v ./skia/out/Release/libskia_effects.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_core.a ]; then 
    cp -v ./skia/out/Release/libskia_core.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_sfnt.a ]; then 
    cp -v ./skia/out/Release/libskia_sfnt.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_images.a ]; then 
    cp -v ./skia/out/Release/libskia_images.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_opts_ssse3.a ]; then 
    cp -v ./skia/out/Release/libskia_opts_ssse3.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libskia_opts.a ]; then 
    cp -v ./skia/out/Release/libskia_opts.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libSDL2.a ]; then 
    cp -v ./SDL2/build/build/.libs/libSDL2.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libcares.a ]; then 
    cp -v ./c-ares/.libs/libcares.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libzita-resampler.a ]; then 
    cp -v ./zita-resampler/libs/libzita-resampler.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libzip.a ]; then 
    cp -v ./libzip/lib/.libs/libzip.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libportaudio.a ]; then 
    cp -v ./portaudio/lib/.libs/libportaudio.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libnspr4.a ]; then 
    cp -v ./mozilla-central/nsprpub/dist/lib/libnspr4.a $LIBS_DIR 
fi

if [ ! -e $LIBS_DIR/libjsoncpp.a ]; then
    cp -v jsoncpp/$JSONCPP_LIB $LIBS_DIR/libjsoncpp.a
fi

if [ ! -e $LIBS_DIR/libhttp_parser.a ]; then
    cp -v ./http-parser/libhttp_parser.a $LIBS_DIR
fi

if [ ! -e $OUTPUT_DIR/libjs_static.a ]; then
    cp -v ./mozilla-central/js/src/libjs_static.a $OUTPUT_DIR
fi

echo "symlinking library to output dir"

CURRENT_DIR=`pwd`
cd $OUTPUT_DIR
ln -sfv ../.libs/libavcodec.a  
ln -sfv ../.libs/libswresample.a  
ln -sfv ../.libs/libavutil.a  
ln -sfv ../.libs/libavformat.a  
ln -sfv ../.libs/libswscale.a  
ln -sfv ../.libs/libskia_pdf.a  
ln -sfv ../.libs/libskia_ports.a  
ln -sfv ../.libs/libskia_skgr.a  
ln -sfv ../.libs/libskia_gr.a  
ln -sfv ../.libs/libcityhash.a  
ln -sfv ../.libs/libskia_utils.a  
ln -sfv ../.libs/libskia_effects.a  
ln -sfv ../.libs/libskia_core.a  
ln -sfv ../.libs/libskia_sfnt.a  
ln -sfv ../.libs/libskia_images.a  
ln -sfv ../.libs/libskia_opts_ssse3.a  
ln -sfv ../.libs/libskia_opts.a  
ln -sfv ../.libs/libSDL2.a  
ln -sfv ../.libs/libcares.a  
ln -sfv ../.libs/libzita-resampler.a  
ln -sfv ../.libs/libzip.a  
ln -sfv ../.libs/libportaudio.a  
ln -sfv ../.libs/libnspr4.a  
ln -sfv ../.libs/libhttp_parser.a  
ln -sfv ../.libs/libjsoncpp.a  
cd $CURRENT_DIR;

echo "All done!" 
