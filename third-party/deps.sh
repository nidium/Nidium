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

function lnlibs {
    CURRENT_DIR=`pwd`
    cd $OUTPUT_DIR
    echo "Making symlink for $1"
    ln -sf ../.libs/$1
    cd $CURRENT_DIR
}


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
    cd jsoncpp/
    curl $DEPS_URL/scons-local-2.2.0.tar.gz | tar zx
    cd ../
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
    curl $DEPS_URL/pa_snapshot.tgz |tar zx
fi

if [ -d "zita-resampler" ]; then
    echo "zita-resampler already downloaded"
else
    echo "Downloading zita-resampler..."
    curl $DEPS_URL/zita-resampler-1.3.0.tar.bz2 |tar xj && mv zita-resampler-1.3.0 zita-resampler
    echo "Patching zita-resampler Makefile..."
    patch -p0 < zita.patch
fi

if [ -d "gyp" ]; then
    echo "gyp already downloaded"
else 
    echo "Downloading gyp central..."
    curl $DEPS_URL/gyp.tar.gz| tar zx
fi

#Build phase
echo "Everything download. Lets build!" 

if [ -e $OUTPUT_DIR/libjs_static.a ]; then
    echo "Mozilla central already build"
else
    echo "Building nspr from mozilla central..."
    cd mozilla-central/nsprpub/
    if [[ $OSX ]]; then
        ./configure --enable-64bit 
    else
        ./configure 
    fi
    make -j$NBCPU
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

    cp -v ./mozilla-central/js/src/libjs_static.a $OUTPUT_DIR
    cp -v ./mozilla-central/nsprpub/dist/lib/libnspr4.a $LIBS_DIR 

    lnlibs libnspr4.a
fi

if [[ $OSX ]]; then
    if [ ! -d $LIBS_DIR/SDL2.framework ]; then 
        cd SDL2/
        cd Xcode/SDL/
        xcodebuild -configuration 'Release' CONFIGURATION_BUILD_DIR='out' -target 'Framework'
        cd ../../../

        cp -rv ./SDL2/Xcode/SDL/out/SDL2.framework $LIBS_DIR 
        lnlibs SDL2.framework
    fi
else
    if [ ! -e $LIBS_DIR/libSDL2.a ]; then 
        cd SDL2/
        mkdir -p build/ && cd build/
        ../configure && make -j$NBCPU
        cd ../../

        cp -v ./SDL2/build/build/.libs/libSDL2.a $LIBS_DIR 
        lnlibs libSDL2.a
    fi
fi

if [ -e $LIBS_DIR/libskia_core.a ]; then
    echo "Skia already build"
else
    echo "Building skia..."
    cd skia 
    if [[ $OSX ]]; then
        export GYP_DEFINES='skia_arch_width=64'
    else
        export GYP_DEFINES='skia_arch_width=32'
    fi
    ./gyp_skia
    make tests BUILDTYPE=Release -j$NBCPU

    cd ../

    cp -v ./skia/out/Release/libskia_pdf.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_ports.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_skgr.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_gr.a $LIBS_DIR 
    cp -v ./skia/out/Release/libcityhash.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_utils.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_effects.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_core.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_sfnt.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_images.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_opts_ssse3.a $LIBS_DIR 
    cp -v ./skia/out/Release/libskia_opts.a $LIBS_DIR 

    lnlibs libskia_pdf.a
    lnlibs libskia_ports.a  
    lnlibs libskia_skgr.a  
    lnlibs libskia_gr.a  
    lnlibs libcityhash.a  
    lnlibs libskia_utils.a  
    lnlibs libskia_effects.a  
    lnlibs libskia_core.a  
    lnlibs libskia_sfnt.a  
    lnlibs libskia_images.a  
    lnlibs libskia_opts_ssse3.a  
    lnlibs libskia_opts.a  
fi

if [ -e $LIBS_DIR/libjsoncpp.a ]; then
    echo "jsoncpp already build"
else
    cd jsoncpp/
    echo "Running scons and building jsoncpp..."
    python scons.py platform=linux-gcc CC="clang" CXX="clang++" CXXFLAGS='-stdlib=libc++' LIBS='-lc++'
    JSONCPP_DIR=libs/`ls -1 libs/ |grep linux-gcc`
    JSONCPP_LIB=`ls -1 $JSONCPP_DIR/libjson_linux-gcc-*_libmt.a`
    cd ../

    cp -v jsoncpp/$JSONCPP_LIB $LIBS_DIR/libjsoncpp.a
    lnlibs libjsoncpp.a
fi

if [ -e $LIBS_DIR/libcares.a ]; then
    echo "c-ares already build"
else
    echo "Building c-ares..."
    cd c-ares && ./configure && make -j$NBCPU
    cd ../

    cp -v ./c-ares/.libs/libcares.a $LIBS_DIR 
    lnlibs libcares.a
fi

if [ -e $LIBS_DIR/libzip.a ]; then
    echo "libzip already build"
else 
    echo "Building libzip...."
    cd libzip && ./configure && make -j$NBCPU
    cd ../

    cp -v ./libzip/lib/.libs/libzip.a $LIBS_DIR 
    lnlibs libzip.a
fi

if [ -e $LIBS_DIR/libhttp_parser.a ]; then
    echo "libhttp_parser already build"
else
    echo "Building libhttp_parser...."
    cd http-parser
    make
    make package
    cd ../

    cp -v ./http-parser/libhttp_parser.a $LIBS_DIR
    lnlibs libhttp_parser.a
fi

if [ -e $LIBS_DIR/libavcodec.a ]; then
    echo "FFmpeg already build"
else
echo "Building ffmpeg...."
    cd ffmpeg 
    ./configure 
       # --disable-avdevice --disable-postproc --disable-avfilter --disable-programs --disable-ffplay --disable-ffserver --disable-ffprobe --disable-everything \
       # --enable-decoder=aac,mp3,vorbis,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,h264,mpeg4,mpeg2video\
       # --enable-parser=vorbis,mpegaudio,mpegvideo,mpeg4video,h264,vp8,aac \
       # --enable-demuxer=mp3,ogg,vorbis,pcm_alaw,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_mulaw,pcm_s16be,pcm_s16le,pcm_s24be,pcm_s24le,pcm_s32be,pcm_s32le,pcm_s8,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,h264,mpegvideo,aac
    make -j$NBCPU

    cd ../

    cp -v ./ffmpeg/libavcodec/libavcodec.a $LIBS_DIR 
    cp -v ./ffmpeg/libswresample/libswresample.a $LIBS_DIR 
    cp -v ./ffmpeg/libavutil/libavutil.a $LIBS_DIR 
    cp -v ./ffmpeg/libavformat/libavformat.a $LIBS_DIR 
    cp -v ./ffmpeg/libswscale/libswscale.a $LIBS_DIR 

    lnlibs libavcodec.a  
    lnlibs libswresample.a  
    lnlibs libavutil.a  
    lnlibs libavformat.a  
    lnlibs /libswscale.a  
fi

if [ -e $LIBS_DIR/libportaudio.a ]; then
    echo "Portaudio already build"
else
    echo "Building portaudio..."
    cd portaudio && ./configure && make -j$NBCPU
    cd ../

    cp -v ./portaudio/lib/.libs/libportaudio.a $LIBS_DIR 
    lnlibs libportaudio.a
fi

if [ -e $LIBS_DIR/libzita-resampler.a ]; then
    echo "zita-resampler already build"
else
    OLD_CXX=$CXX
    CXX=/usr/bin/clang++
    echo "Building zita-resampler..."
    cd zita-resampler/libs/ && make -j$NBCPU
    cd ../../

    cp -v ./zita-resampler/libs/libzita-resampler.a $LIBS_DIR 
    lnlibs libzita-resampler.a
    CXX=$OLD_CXX
fi

echo "All done!" 
