#!/usr/bin/python
import sys, os, imp
sys.path.append(".")
sys.path.append("tools")

from konstructor import Konstruct
from konstructor import Builder
from konstructor import Platform
from konstructor import Utils
from konstructor import Log

imp.load_source("configure", "configure");

Gyp = Builder.Gyp

Konstruct.setConfigs(["release"])
Gyp.setConfiguration("Release")

Gyp.set("native_enable_breakpad", 1)

OUTPUT_BINARY = os.path.join("build", "out", Gyp._config, "nidium")

# {{{ Release Utilities
def release():
    import zipfile 
    import urllib2, mimetypes
    import subprocess

    # This is a little hack to be able to display upload progress
    class fakeFile():
        def __init__(self, data):
            self._data = data
            self._total = len(data)
            self._pos = 0
            self._args = 'fakeFile'

        def __len__(self):
            return self._total

        def read(self, size):
            if self._pos > self._total:
                self._pos = self._total;

            data = self._data[self._pos:self._pos + size]

            status = r"%10d  [%3.2f%%]" % (self._pos, self._pos * 100. / self._total)
            status = status + chr(8)*(len(status)+1)
            Log.info(status);
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

    symFile = ""
    binFile = ""
    breakpadSymFile = ""

    if Platform.system == "Darwin":
        breakpadSymFile = "gyp/build/" + Gyp._config + "/nidium.sym"
        symFile = "gyp/build/Release/nidium.app.dSYM/Contents/Resources/DWARF/nidium"
        binFile = "framework/dist/nidium.app/Contents/MacOS/nidium"
    elif Platform.system == "Linux":
        breakpadSymFile = "build/out/" + Gyp._config + "/nidium.sym"
        symFile = "build/out/" + Gyp._config + "/nidium.debug"
        binFile = OUTPUT_BINARY
    else:
        # Window TODO
        print("TODO")

    Log.info("Producing debug symbols")

    if Platform.system == "Darwin":
        Utils.run("tools/dump_syms " + symFile + " > " + breakpadSymFile)
    elif Platform.system == "Linux":
        Utils.run("objcopy --only-keep-debug " + binFile + " " + symFile)
        Utils.run("objcopy --add-gnu-debuglink " + symFile+ " " + binFile)
        Utils.run("tools/dump_syms " + binFile + "  > " + breakpadSymFile)
    else:
        # Window TODO
        print("TODO")

    Log.info("Archiving debug symbols")
    symArchive = "build/out/nidium.sym.zip"

    with zipfile.ZipFile(symArchive, 'w', zipfile.ZIP_DEFLATED) as myzip:
        myzip.write(breakpadSymFile, "nidium.sym")
        myzip.write(symFile, "nidium.debug")

    stripExecutable()
    packageExecutable()

    Log.info("Uploading application symbols. Bytes : %s " % (os.stat(symArchive).st_size));
    symbols = open(symArchive, "rb").read()
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
    arch = ""
    if Platform.wordSize == 64:
        arch = "x86_64"
    else:
        arch = "i386"
    reply = post_multipart("crash.nidium.com", "/upload_symbols", [["build", revision], ["arch", arch], ["system", Platform.system]], [["symbols", "nidium.sym.zip", symbols]])

    if reply.strip() != "OK":
        Utils.exit("Failed to upload symbols : " + reply)

    os.unlink(symFile)
    os.unlink(symArchive)
    os.unlink(breakpadSymFile)


def stripExecutable():
    Log.info("Striping executable")
    if Platform.system == "Darwin":
        Utils.run("strip framework/dist/nidium.app/Contents/MacOS/nidium")
        return
    elif Platform.system == "Linux":
        Utils.run("strip " + OUTPUT_BINARY)
    else:
        # Window TODO
        print("TODO")


def signCode(path):
    import subprocess

    Log.info("Signing object...")

    identity = "Anthony Catel"

    code, output = Utils.run(" ".join([
        "codesign",
        "--force",
        "--sign",
        "'Developer ID Application: %s '" % identity,
        path
    ]), failExit=False)

    if code != 0:
        Log.error("WARNING : App signing failed with identity %s. Not signing app" % identity)
        Log.error(output)

    Log.info(path)

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

    if Platform.wordSize == 64:
        arch = "x86_64"
    else:
        arch = "i386"

    if tag is None:
        datetime = time.strftime("%Y%m%d_%H%M%S")
        name = "Nidium_%s_%s_%s_%s" % (datetime, revision, Platform.system, arch)
    else:
        name = "Nidium_%s_%s_%s" % (tag, Platform.system, arch)

    Log.info("Packaging executable")

    if Platform.system == "Darwin":

        signCode(path + "nidium.app")

        Log.info("Create dmg...")

        resources += "osx/"
        name += ".dmg"
        cmd = [
            "tools/installer/osx/create-dmg",
            "--volname 'Nidium'",
            "--no-internet-enable",
            "--volicon " + resources + "/nidium.icns",
            "--background " + resources + "/dmg-background.png",
            "--window-size 555 394",
            "--icon-size 96",
            "--eula ./COPYING",
            "--app-drop-link 460 290",
            "--icon 'nidium' 460 80",
            "out/" + name,
            path + "nidium.app/"
        ]
        code, output = Utils.run(" ".join(cmd))
        if code != 0:
            Utils.exit("Failed to build dmg")
    elif Platform.system == "Linux":
        import shutil
        resources += "linux/"
        name += ".run"
        tmpDir = "out/nidium.tmp/"
        Utils.mkdir(tmpDir + "dist/")
        Utils.mkdir(tmpDir + "resources/")
        shutil.copy("./COPYING", tmpDir)
        shutil.copy(resources + "/nidium.desktop", tmpDir + "resources/")
        shutil.copy(resources + "/x-application-nidium.xml", tmpDir + "resources/")
        shutil.copy(resources + "/nidium.png", tmpDir + "resources/")
        shutil.copy(resources + "/installer.sh", tmpDir)
        shutil.copy(path + "nidium",  tmpDir + "dist/")
        shutil.copy(path + "nidium-crash-reporter", tmpDir + "dist/")

        Utils.run("tools/installer/linux/makeself.sh %s out/%s 'Nidium installer' ./installer.sh " % (tmpDir, name))

        shutil.rmtree(tmpDir);

    else:
        # Window TODO
        print("TODO")

    uploadExecutable("out/", name)

def uploadExecutable(path, name):
    import ftplib
    from pprint import pprint
    
    args = {"pos": 0, "total": os.stat(path + name).st_size} 

    def callback(data):
        args["pos"] += 1024 

        status = r"%10d  [%3.2f%%]" % (args["pos"], args["pos"] * 100. / args["total"])
        status = status + chr(8)*(len(status)+1)
        Log.info(status)

    Log.info("Uploading executable")
    s = ftplib.FTP("nidium.com", "nidium", "i8V}8B833G51gZJ")
    f = open(path + name, 'rb')
    s.storbinary("STOR release/" + name, f, 1024, callback)

    #os.unlink(path + name);
    Log.info("Executable uploaded to http://release.nidium.com/" + name)
# }}}

if not os.path.exists("tools/dir2nvfs"):
    # In Release mode, we need to embed the private
    # Dir2NFS is needed in order to generate the privates
    Gyp("gyp/tools.gyp").run("dir2nvfs")

Gyp("gyp/actions.gyp").run("generate-private", parallel=False)

# Now that the privates are build
# we can add add native_embed_private flag
Gyp.set("native_embed_private", 1)

if __name__ == '__main__':
    Konstruct.start() 
    release()
