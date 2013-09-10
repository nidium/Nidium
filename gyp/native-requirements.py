from functools import partial
import sys, os
import deps

def makePreload():
    import re

    cwd = os.getcwd()
    os.chdir(deps.CWD)

    deps.logd("Building preload.h")
    os.chdir("scripts")

    inFile = open("preload.js", "r")
    outFile = open("../src/NativeJS_preload.h", "w")

    data = inFile.read()

    data = re.sub('"', '\\"', data, flags=re.MULTILINE)
    data = re.sub('^', '"', data, flags=re.MULTILINE)
    data = re.sub('$', '\\\\n"', data, flags=re.MULTILINE)
    data = "const char *preload_js = " + data + ";"

    outFile.write(data)

    inFile.close()
    outFile.close()

    os.chdir(cwd)

def buildSDL2():
    if deps.system == "Darwin":
        deps.buildDep("SDL2.framework", "SDL2/Xcode/SDL", ["xcodebuild -configuration 'Release' CONFIGURATION_BUILD_DIR='out' -target 'Framework'"], outlibs=["SDL2/Xcode/SDL/out/SDL2.framework"])
    else:
        deps.mkdir_p("SDL2/build/")
        deps.buildDep("libSDL2", "SDL2/build/", ["../configure", "make"], outlibs=["SDL2/build/build/.libs/libSDL2"])

def buildSkia():
    #deps.patchDep("skia", "skia-addPath-new-arg.patch")

    exports = "GYP_DEFINES='skia_arch_width=32'"

    if deps.is64bits:
        exports = "GYP_DEFINES='skia_arch_width=64'"

    deps.buildDep("libskia_core", "skia", [exports + " ./gyp_skia", exports + " make tests BUILDTYPE=Release -j " + str(deps.nbCpu)], outlibs=[
        "skia/out/Release/libskia_pdf",
        "skia/out/Release/libskia_ports",
        "skia/out/Release/libskia_skgpu",
        "skia/out/Release/libskia_utils",
        "skia/out/Release/libskia_effects",
        "skia/out/Release/libskia_core",
        "skia/out/Release/libskia_sfnt",
        "skia/out/Release/libskia_images",
        "skia/out/Release/libskia_opts_ssse3",
        "skia/out/Release/libskia_opts"
    ])

def buildJSONCPP():
    if deps.system == "Linux" or deps.system == "Darwin":
        platform = "linux-gcc"

        # Dirty hack to get output directory/name of libjsoncpp
        import commands 
        version = commands.getoutput('g++ -dumpversion')
        outlib = ["jsoncpp/libs/linux-gcc-" + version + "/libjson_linux-gcc-" + version + "_libmt", "libjsoncpp"]
    else :
        platform = "msvc8"

    deps.buildDep("libjsoncpp", "jsoncpp", ["python scons.py platform=" + platform + "  CC='clang' CXX='clang++' CXXFLAGS='-stdlib=libc++' LIBS='-lc++'"], outlibs=[outlib]);

def downloadJSONCPP():
    deps.downloadDep("jsoncpp", deps.depsURL + "/jsoncpp-src-0.5.0.tar.gz", "jsoncpp-src*")
    os.chdir("jsoncpp")
    # XXX : This is a hack, scons-local should be embedded in jsoncpp archive
    deps.downloadDep("jsoncpp", deps.depsURL + "/scons-local-2.2.0.tar.gz")
    os.chdir("../")

def buildZitaResampler():
    deps.patchDep("zita-resampler", "zita.patch")
    deps.buildDep("libzita-resampler", "zita-resampler/libs/", ["make"], outlibs=["zita-resampler/libs/libzita-resampler"])

def buildLibCoroutine():
    deps.patchDep("libcoroutine", "libcoroutine.patch")
    if deps.BUILD == "debug":
        deps.patchDep("libcoroutine", "libcoroutine.debug.patch")

    deps.buildDep("libcoroutine", "libcoroutine", ["make"], outlibs=[["libcoroutine/_build/lib/liblibcoroutine", "libcoroutine"]])

def registerDeps():
    deps.registerDep("ffmpeg",
        partial(deps.downloadDep, "ffmpeg", deps.depsURL + "/ffmpeg-snapshot.tar.bz2"),
        partial(deps.buildDep, "libavcodec", "ffmpeg", ["./configure \
        --disable-avdevice --disable-postproc --disable-avfilter --disable-programs --disable-ffplay --disable-ffserver --disable-ffprobe --disable-everything \
        --enable-decoder=aac,mp3,vorbis,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,h264,mpeg4,mpeg2video\
        --enable-parser=vorbis,mpegaudio,mpegvideo,mpeg4video,h264,vp8,aac \
        --enable-demuxer=mp3,ogg,vorbis,pcm_alaw,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_mulaw,pcm_s16be,pcm_s16le,pcm_s24be,pcm_s24le,pcm_s32be,pcm_s32le,pcm_s8,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,h264,mpegvideo,aac,mov,avi,wav", "make"], outlibs=[
    "ffmpeg/libavcodec/libavcodec",
    "ffmpeg/libswresample/libswresample",
    "ffmpeg/libavutil/libavutil",
    "ffmpeg/libavformat/libavformat",
    "ffmpeg/libswscale/libswscale"]))

    deps.registerDep("basekit",
        partial(deps.downloadDep, "basekit", deps.depsURL + "/libbasekit.zip", "stevedekorte-basekit"),
        partial(deps.buildDep, "libbasekit", "basekit", ["make"], outlibs=["basekit/_build/lib/libbasekit"]))

    deps.registerDep("libcoroutine",
        partial(deps.downloadDep, "libcoroutine", deps.depsURL + "/libcoroutine.zip", "stevedekorte-coroutine"),
        buildLibCoroutine)

    deps.registerDep("portaudio",
        partial(deps.downloadDep, "portaudio", deps.depsURL + "/pa_snapshot.tgz"),
        partial(deps.buildDep, "libportaudio", "portaudio", ["./configure", "make"], outlibs=["portaudio/lib/.libs/libportaudio"]))

    deps.registerDep("zita-resampler",
        partial(deps.downloadDep, "zita-resampler", deps.depsURL + "/zita-resampler-1.3.0.tar.bz2", "zita-resampler-1.3.0"),
        buildZitaResampler)

    deps.registerDep("SDL2",
        partial(deps.downloadDep, "SDL2", deps.depsURL + "/SDL-2.0.tar.gz", "SDL-2*"),
        buildSDL2)

    deps.registerDep("skia", 
        partial(deps.downloadDep, "skia", deps.depsURL + "/skia.tar.gz"),
        buildSkia)

    deps.registerDep("libzip",
        partial(deps.downloadDep, "libzip", deps.depsURL + "/libzip-0.10.1.tar.bz2", "libzip-0.10.1"),
        partial(deps.buildDep, "libzip", "libzip", ["./configure", "make"], outlibs=["libzip/lib/.libs/libzip"]))

    deps.registerDep("jsoncpp", 
        downloadJSONCPP, 
        buildJSONCPP)

    deps.registerDep("preload", 
         None,
         makePreload)
