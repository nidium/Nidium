from functools import partial
import sys, os
import deps
from deps import log, spinner

def buildSDL2():
    if deps.system == "Darwin":
        deps.buildDep("SDL2.framework", "SDL2/Xcode/SDL", ["xcodebuild -configuration 'Release' CONFIGURATION_BUILD_DIR='out' -target 'Framework'"], outlibs=["SDL2/Xcode/SDL/out/SDL2.framework"])
    else:
        deps.mkdir_p("SDL2/build/")
        deps.buildDep("libSDL2", "SDL2/build/", ["../configure", "make"], outlibs=["SDL2/build/build/.libs/libSDL2"])

def downloadSkia():
    if deps.needDownload("skia", "skia"):
        log.step("Downloading skia")
        deps.runCommand("depot_tools/gclient sync --gclientfile=gclient_skia")

def buildSkia():
    gypDefines = os.environ.get('GYP_DEFINES')
    if deps.is64bits:
        exports = "GYP_DEFINES='skia_arch_width=64"
    else:
        exports = "GYP_DEFINES='skia_arch_width=32"
    if gypDefines is not None:
        exports += " " + gypDefines

    exports += "'"

    makeFlags = ""
    if deps.VERBOSE:
        makeFlags = "V=1"

    deps.buildDep("libskia_core", "skia", [exports + " ./gyp_skia -I../../gyp/skia.gypi", exports + " make tests BUILDTYPE=Release " + makeFlags + " -j " + str(deps.nbCpu)], outlibs=[
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

def buildZitaResampler():
    deps.patchDep("zita-resampler", "zita.patch")
    deps.buildDep("libzita-resampler", "zita-resampler/libs/", ["make"], outlibs=["zita-resampler/libs/libzita-resampler"])

def buildLibCoroutine():
    deps.patchDep("libcoroutine", "libcoroutine.patch")
    if deps.BUILD == "debug" and deps.ENABLE_VALGRIND:
        deps.patchDep("libcoroutine", "libcoroutine.debug.patch")

    deps.buildDep("libcoroutine", "libcoroutine", ["make"], outlibs=[["libcoroutine/_build/lib/liblibcoroutine", "libcoroutine"]])

def buildLevelDB():
    flags = ""
    if deps.system == "Darwin":
        flags = "CXXFLAGS='-stdlib=libc++ -mmacosx-version-min=10.7' CFLAGS='-mmacosx-version-min=10.7'"

    deps.buildDep("libleveldb", "leveldb", [flags + " make"], outlibs=["leveldb/libleveldb"])

def registerDeps():
    deps.registerDep("depot_tools",
        partial(deps.downloadDep, "depot_tools", deps.depsURL + "/depot_tools.tar.gz"),
        None)

    deps.registerDep("ffmpeg",
        partial(deps.downloadDep, "ffmpeg", deps.depsURL + "/ffmpeg-snapshot.tar.bz2"),
        partial(deps.buildDep, "libavcodec", "ffmpeg", ["./configure \
        --disable-avdevice --disable-postproc --disable-avfilter --disable-programs --disable-ffplay --disable-ffserver --disable-ffprobe --disable-everything \
        --enable-decoder=ac3,aac,mp3,vorbis,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,h264,mpeg4,mpeg2video\
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
        partial(deps.downloadDep, "portaudio", deps.depsURL + "/portaudio.tgz"),
        partial(deps.buildDep, "libportaudio", "portaudio", ["./configure", "make"], outlibs=["portaudio/lib/.libs/libportaudio"]))

    deps.registerDep("zita-resampler",
        partial(deps.downloadDep, "zita-resampler", deps.depsURL + "/zita-resampler-1.3.0.tar.bz2", "zita-resampler-1.3.0"),
        buildZitaResampler)

    deps.registerDep("SDL2",
        partial(deps.downloadDep, "SDL2", deps.depsURL + "/SDL-2.0.1-8054.tar.gz", "SDL-2*"),
        buildSDL2)

    deps.registerDep("leveldb",
        partial(deps.downloadDep, "leveldb", deps.depsURL + "/leveldb.tar.gz"),
        buildLevelDB)

    deps.registerDep("skia", 
        downloadSkia,
        buildSkia)

    deps.registerDep("libzip",
        partial(deps.downloadDep, "libzip", deps.depsURL + "/libzip-0.10.1.tar.bz2", "libzip-0.10.1"),
        partial(deps.buildDep, "libzip", "libzip", ["./configure", "make"], outlibs=["libzip/lib/.libs/libzip"]))

    deps.registerDep("angle",
        partial(deps.downloadDep, "angle", deps.depsURL + "/angle.tar.gz"),
        None)

    deps.registerDep("breakpad",
        partial(deps.downloadDep, "breakpad", deps.depsURL + "/breakpad.tar.gz"),
        None)
        #partial(deps.buildDep, "libbreakpad_client", "breakpad", ["./configure", "make"], outlibs=["breakpad/src/client/linux/libbreakpad_client"]))

    deps.registerAction(releaseActionRegister, releaseActionParse, releaseAction);
    deps.registerAction(stripActionRegister, None, stripAction)

def stripActionRegister(parser):
    parser.add_option("--strip", dest="strip", action="store_true", default=False, help="Strip executable")

def stripAction(options):
    if options.strip:
        stripExecutable()

def releaseActionParse(options):
    if options.release:
        deps.gypArgs += " -Dnative_enable_breakpad=1"

def releaseActionRegister(parser):
    parser.add_option("--release", dest="release", action="store_true", default=False, help="Publish a release")

def releaseAction(opt):
    import os
    import zipfile 
    import urllib2, mimetypes
    import subprocess

    if opt.release is False:
        return

    # This is a little hack to be able to display upload progress
    class fakeFile():
        def __init__(self, data):
            self._data = data
            self._total = len(data)
            self._pos = 0
            self._args = 'fakeFile'
            self._gen = deps.spinningCursor()

        def __len__(self):
            return self._total

        def read(self, size):
            if self._pos > self._total:
                self._pos = self._total;

            data = self._data[self._pos:self._pos + size]

            c = next(self._gen)
            status = r"%10d  [%3.2f%%]" % (self._pos, self._pos * 100. / self._total)
            status = status + chr(8)*(len(status)+1)
            log.spinner(c, status)
            sys.stdout.flush()

            self._pos += size

            return data 

    # Utilities function to post multipart form data 
    # inspired from http://code.activestate.com/recipes/146306/
    def post_multipart(host, selector, fields, files):
        content_type, body = encode_multipart_formdata(fields, files)
        stream = fakeFile(body)
        ret = ""
        try:
            req = urllib2.Request("http://" + host + selector, stream, {"Content-Type": content_type})
            res = urllib2.urlopen(req)
            ret = res.read()
        except urllib2.HTTPError as err:
            ret = err.read()

        return ret

    def encode_multipart_formdata(fields, files):
        BOUNDARY = '----------ThIs_Is_tHe_bouNdaRY_$'
        CRLF = '\r\n'
        L = []
        for (key, value) in fields:
            L.append('--' + BOUNDARY)
            L.append('Content-Disposition: form-data; name="%s"' % key)
            L.append('')
            L.append(value)
        for (key, filename, value) in files:
            L.append('--' + BOUNDARY)
            L.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (key, filename))
            L.append('Content-Type: %s' % get_content_type(filename))
            L.append('')
            L.append(value)
        L.append('--' + BOUNDARY + '--')
        L.append('')
        body = CRLF.join(L)
        content_type = 'multipart/form-data; boundary=%s' % BOUNDARY
        return content_type, body

    def get_content_type(filename):
        return mimetypes.guess_type(filename)[0] or 'application/octet-stream'

    breakpadSymFile = "out/nidium.sym"
    symFile = ""
    binFile = ""
    if deps.system == "Darwin":
        symFile = "gyp/build/Release/nidium.app.dSYM/Contents/Resources/DWARF/nidium"
        binFile = "framework/dist/nidium.app/Contents/MacOS/nidium"
    elif deps.system == "Linux":
        symFile = "framework/dist/nidium.debug"
        binFile = "framework/dist/nidium"
    else:
        # Window TODO
        print("TODO")

    log.step("Producing debug symbols")

    if deps.system == "Darwin":
        deps.runCommand("tools/dump_syms " + symFile + " > " + breakpadSymFile)
    elif deps.system == "Linux":
        deps.runCommand("objcopy --only-keep-debug " + binFile + " " + symFile)
        deps.runCommand("objcopy --add-gnu-debuglink " + symFile+ " " + binFile)
        deps.runCommand("tools/dump_syms " + binFile + "  > " + breakpadSymFile)
    else:
        # Window TODO
        print("TODO")

    log.step("Archiving debug symbols")
    symArchive = "out/nidium.sym.zip"
    log.info(symArchive)

    spinner.start()

    with zipfile.ZipFile(symArchive, 'w', zipfile.ZIP_DEFLATED) as myzip:
        myzip.write(breakpadSymFile, "nidium.sym")
        myzip.write(symFile, "nidium.debug")

    spinner.stop()
    log.setOk()

    log.step("Uploading application symbols. Bytes : %s " % (os.stat(symArchive).st_size));
    symbols = open(symArchive, "rb").read()
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
    arch = ""
    if deps.is64bits:
        arch = "x86_64"
    else:
        arch = "i386"
    reply = post_multipart("crash.nidium.com", "/upload_symbols", [["build", revision], ["arch", arch], ["system", deps.system]], [["symbols", "nidium.sym.zip", symbols]])

    if reply.strip() == "OK":
        print ""
        log.setOk()
    else:
        log.setError()
        log.error("Failed to upload symbols : " + reply)
        sys.exit(3)


    os.unlink(symFile)
    os.unlink(symArchive)
    os.unlink(breakpadSymFile)

    stripExecutable()
    packageExecutable()

def stripExecutable():
    log.step("Striping executable")
    if deps.system == "Darwin":
        deps.runCommand("strip framework/dist/nidium.app/Contents/MacOS/nidium")
        return
    elif deps.system == "Linux":
        deps.runCommand("strip framework/dist/nidium")
    else:
        # Window TODO
        print("TODO")

def packageExecutable():
    import time
    import subprocess
    import tarfile 

    revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
    tag = None
    path = "framework/dist/"
    resources = "resources/"
    arch = ""
    name = ""

    try:
        tag = subprocess.check_output(["git", "describe",  "--exact-match", revision], stderr=subprocess.PIPE)
        print "Tag is " + tag
    except:
        tag = None

    if deps.is64bits:
        arch = "x86_64"
    else:
        arch = "i386"

    if tag is None:
        datetime = time.strftime("%Y%m%d_%H%M%S")
        name = "Nidium_%s_%s_%s_%s" % (datetime, revision, deps.system, arch)
    else:
        name = "Nidium_%s_%s_%s" % (tag, deps.system, arch)

    log.step("Packaging executable")
    log.info(name)
    spinner.start()

    if deps.system == "Darwin":
        resources += "osx/"
        name += ".dmg"
        cmd = [
            "tools/installer/osx/create-dmg", 
            "--volname", "Nidium", 
            "--volicon", resources + "/nidium.icns",
            "--background", resources + "/dmg-background.png",
            "--app-drop-link", "100 100",
            "out/" + name,
            path + "nidium.app/"
        ]
        code, output = deps.runCommand(" ".join(cmd))
        if code != 0:
            log.setError()
            log.error("Failed to build dmg")
            sys.exit(3)
    elif deps.system == "Linux":
        import shutil
        resources += "linux/"
        name += ".run"
        tmpDir = "out/nidium.tmp/"
        deps.mkdir_p(tmpDir + "dist/")
        deps.mkdir_p(tmpDir + "resources/")
        shutil.copy(resources + "/nidium.desktop", tmpDir + "resources/")
        shutil.copy(resources + "/x-application-nidium.xml", tmpDir + "resources/")
        shutil.copy(resources + "/nidium.png", tmpDir + "resources/")
        shutil.copy(resources + "/installer.sh", tmpDir)
        shutil.copy(path + "nidium",  tmpDir + "dist/")
        shutil.copy(path + "nidium-crash-reporter", tmpDir + "dist/")

        deps.runCommand("tools/installer/linux/makeself.sh %s out/%s 'Nidium installer' ./installer.sh " % (tmpDir, name))

        shutil.rmtree(tmpDir);

    else:
        # Window TODO
        print("TODO")

    spinner.stop()
    log.setOk()

    uploadExecutable("out/", name)

def uploadExecutable(path, name):
    import ftplib
    from pprint import pprint
    
    args = {"gen": deps.spinningCursor(), "pos": 0, "total": os.stat(path + name).st_size} 

    def callback(data):
        args["pos"] += 1024 

        c = next(args["gen"])
        status = r"%10d  [%3.2f%%]" % (args["pos"], args["pos"] * 100. / args["total"])
        status = status + chr(8)*(len(status)+1)
        log.spinner(c, status)

    log.step("Uploading executable")

    s = ftplib.FTP("nidium.com", "nidium", "i8V}8B833G51gZJ")
    f = open(path + name, 'rb')
    s.storbinary("STOR release/" + name, f, 1024, callback)

    print ""
    os.unlink(path + name);
    log.setOk()
    log.info("Executable uploaded to http://release.nidium.com/" + name)

