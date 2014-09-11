#!/usr/bin/python
#-*- coding: utf-8 -*-
import platform
import multiprocessing
from optparse import OptionParser
import sys, os
from pprint import pprint

# Variables to configure
OUTPUT = "out/"
THIRD_PARTY = "third-party/"
BUILD = "release"
AUTOCONF = "/usr/bin/autoconf2.13"
GIT = "/usr/bin/git"
GCC = "/usr/bin/gcc"
GPP = "/usr/bin/g++"
CLANG = "/usr/bin/clang"
CLANGPP = "/usr/bin/clang++"

os.environ['MACOSX_DEPLOYMENT_TARGET'] = '10.7'
os.environ['GYP_DEFINES'] = 'skia_resource_cache_mb_limit=256'

# Change to gyp executable path if you don't 
# want this script to download gyp for you
GYP = None 

# TODO : 
# * Auto-detect missing external software (git, autoconf, clang, etc..) and automatically install them
# * Auto-detect path for external software

if __name__ == '__main__':
    # Add deps.py folder location to python path
    # so the file can be imported from requirements modules
    sys.path.append(os.path.realpath(__file__))

    # Self import deps.py module. This is a bit weird but it's needed to get the
    # availableDependencies variables modifed when calling registerDeps() method
    # Because the executed script from the command line do not share 
    # his variables when imported as a module
    import deps as d

    # Setup KeywordInterrupt handler 
    import signal
    signal.signal(signal.SIGINT, d.signalHandler)

    # Quickly read command line arguments and look for requirements file
    if len(sys.argv) > 1:
        requirementsFile = sys.argv[1]
        d.processRequirements(requirementsFile)

    # Register command line arguments, and parse it
    opt, arguments = d.parseArguments()

    #Download git repo
    if opt.init is not None:
        requirementsFile = d.cloneRepo(opt.init)
        # cloneRepo switch to newly cloned repo,
        # CWD need to be updated
        CWD = os.getcwd() 
    else:
        requirementsFile = arguments[0]

    # Create output directories
    d.initDirectories()

    # Download and build everything
    d.log.step("Checking dependencies");
    d.downloadAndBuildDeps()

    # Call pre action (actually these actions are the build phase)
    for action in d.availableActions["pre"]:
        action()

    d.log.success("Build successfull")

    for action in d.availableActions["post"]:
        action(opt)

    d.log.info("Finished \\o/")
    # we don't want to run the rest of the script
    sys.exit()


# Setup environement variables
system = platform.system()
nbCpu = multiprocessing.cpu_count()
depsURL = "http://deps.nidium.com"
is64bits = sys.maxsize > 2**32
gypArgs = ""
optionParser = OptionParser(usage="Usage: %prog [requirements_file] [options]")

availableDependencies = {}
availableOptions = []
availableActions = {"pre": [], "post": [], "parse": []}
deps = []

VERBOSE = False
LIBS_DIR = None
LIBS_OUTPUT = None
FORCE_BUILD = []
FORCE_DOWNLOAD = []
CURRENT_DEP = None
ENABLE_VALGRIND = False
LOG_FILE = open('./deps.log', 'w')

def mkdir_p(path):
    import os, errno
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def initDirectories():
    global OUTPUT, LIBS_DIR, LIBS_OUTPUT, CWD
    OUTPUT = os.path.realpath(OUTPUT) + "/"
    LIBS_DIR = OUTPUT + "/third-party-libs/.libs/"
    LIBS_OUTPUT = OUTPUT + "/third-party-libs/" + BUILD + "/"

    CWD = os.getcwd()

    mkdir_p(OUTPUT + "/third-party-libs/.libs/")
    mkdir_p(OUTPUT + "/third-party-libs/release/")
    mkdir_p(OUTPUT + "/third-party-libs/debug/")

def signalHandler(signal, frame):
    log.setError()
    log.info("User aborted")
    STOP_SPINNER_THREAD = True
    sys.exit(0)

def parseArgumentsFile():
    opt, args = optionParser.parse_args()

    if len(args) > 0:
        return args
    else:
        return None

def parseArguments():
    import sys
    global gypArgs

    optionParser.add_option("--init", dest="init", help="Repository name to initialize (NativeStudio, NativeJSCore, libapenetwork)", metavar="REPOSITORY")
    optionParser.add_option("--verbose", "-v", dest="verbose", action="store_true", default=False, help="Be verbose")
    optionParser.add_option("--debug", dest="debug", action="store_true", default=False, help="Build a debug version")
    optionParser.add_option("--force-build", dest="forceBuild", default=False, help="Force building of dependencies. Eg : --force-build=skia,mozilla-central")
    optionParser.add_option("--force-download", dest="forceDownload", default=False, help="Force download of dependencies. Eg : --force-download=skia,mozilla-central")
    optionParser.add_option("-D", dest="defines", type="string", action="append", help="Defines variables to be passed to gyp\n Eg : -Dfoo=1")
    optionParser.add_option("--enable-valgrind", dest="valgrind", default=False, action="store_true", help="Enable valgrind (only works in debug build)")

    for option in availableOptions:
        option(optionParser)

    opt, args = optionParser.parse_args()

    if opt.defines is not None:
        for define in opt.defines:
            gypArgs += "-D%s " % define

    for action in availableActions["parse"]:
        if action(opt) is True:
            exit(0)

    if opt.verbose is True:
        global VERBOSE
        VERBOSE = True

    if opt.debug is True:
        global BUILD, ENABLE_VALGRIND
        BUILD = "debug"
        if opt.valgrind:
            ENABLE_VALGRIND = True;
    
    if opt.forceBuild:
        global FORCE_BUILD
        FORCE_BUILD = opt.forceBuild.split(",")

    if opt.forceDownload:
        global FORCE_DOWNLOAD
        FORCE_DOWNLOAD = opt.forceDownload.split(",")

    if len(args) == 0 and opt.init is None:
        log.error("You must specify a requirements file")
        optionParser.print_help()
        sys.exit(0)
    
    return opt, args

def registerAction(register, parse = None, post = None, pre = None):
    availableOptions.append(register)

    if parse is not None:
        availableActions["parse"].append(parse)

    if post is not None:
        availableActions["post"].append(post)

    if pre is not None:
        availableActions["pre"].append(pre);

def needDep(dep):
    return dep in deps

def registerDep(name, download, build = None):
    availableDependencies[name] = {"name": name, "download": download, "build":build}

def downloadDep(depName, url, rename = None):
    import urllib2

    file_name = url.split('/')[-1]

    if needDownload(depName, file_name):
        u = urllib2.urlopen(url)
        f = open(file_name, 'wb')
        meta = u.info()
        file_size = int(meta.getheaders("Content-Length")[0])
        log.step("Downloading: %s (%s) Bytes: %s" % (depName, file_name, file_size))

        file_size_dl = 0
        block_sz = 8192
        gen = spinningCursor()
        while True:
            buffer = u.read(block_sz)
            if not buffer:
                break

            file_size_dl += len(buffer)
            f.write(buffer)
            status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
            status = status + chr(8)*(len(status)+1)
            c = next(gen)

            log.spinner(c, status)

            sys.stdout.flush()

        log.log("\n")
        log.setOk()
        f.close()

    if depName in deps and os.path.exists(depName) == False:
            extractDep(file_name)

            # Does the extracted archive need to be renamed?
            if rename is not None:
                import re
                files = os.listdir("./")
                for f in files:
                    if re.match(rename, f) and os.path.isdir(f):
                        log.info("    Renaming : " + f + " to " + depName + "\n")
                        os.rename(f, depName)

            return True

def extractDep(path):
    log.action("Extracting " + path)

    extension = os.path.splitext(path)[1]
    if extension == ".zip":
        from zipfile import ZipFile
        zip = ZipFile(path)
        zip.extractall()
    else:
        import tarfile
        tar = tarfile.open(path)
        tar.extractall()
        tar.close()

    log.setOk()

def needDownload(dep, fileName):
    if CURRENT_DEP in FORCE_DOWNLOAD:
        return True

    if dep in deps:
        if os.path.exists(dep):
            if VERBOSE:
                log.info("Skipping dep " + dep + " (already downloaded)")
            return False
        else:
            return True
    else:
        if VERBOSE:
            log.info("Skipping dep " + dep + " (not needed)")
        return False

def patchDep(dep, patchFile):
    if not os.path.exists(dep):
        log.error("Dependency does not exist. Not patching : " + dep)
        sys.exit(1)

    import subprocess

    cwd = os.getcwd()
    patch = open(patchFile)
    nullout = open(os.devnull, 'w')
    os.chdir(dep)
    applied = subprocess.call(["patch", "-p1", "-N", "--dry-run", "--silent"], stdin=patch, stdout=nullout, stderr=subprocess.STDOUT)

    if applied == 0:
        log.step("    Applying patch " + patchFile + " to " + dep)
        patch.seek(0)
        success, output = runCommand("patch -p1 -N", stdin=patch)
        if success != 0:
            log.error("Failed to patch")
            sys.exit(1)
    elif VERBOSE:
        log.info("    Already applied patch "+ patchFile + " to " + dep + ". Skipping.")

    os.chdir(cwd)
    patch.close()
    nullout.close()

def cloneRepo(repo):
    import subprocess
    import sys

    repoURL = ""
    if repo == "NativeStudio":
        repoURL = "https://github.com/paraboul/NativeStudio.git"
        requirementsFile = "gyp/native-requirements.deps"
    elif repo == "libapenetwork":
        repoURL = "https://github.com/SwelenFrance/libapenetwork"
        requirementsFile = "gyp/libapenetwork-requirements.deps"
    elif repo == "NativeJSCore":
        repoURL = "https://github.com/SwelenFrance/NativeJSCore"
        requirementsFile = "gyp/nativejscore-requirements.deps"
    else:
        log.error("Unknown repo " + repo + ". Exiting.")
        sys.exit()

    if os.path.exists(repo):
        log.info("Already cloned repo " + repo)
        os.chdir(repo)
    else:
        log.debug("Cloning " + repo + " ( " + repoURL + ")")

        cloned = subprocess.call([GIT, "clone", repoURL]);

        if cloned != 0:
            log.error("Failed to clone NativeStudio repo. Exiting")
            sys.exit()

        os.chdir(repo)

        sub = subprocess.call([GIT, "submodule", "update", "--init", "--recursive"]);
        if sub != 0:
            log.error("Failed to init " + repo + " submodule(s). Exiting")
            sys.exit()

    return requirementsFile

def needBuild(depName, symlink):
    if symlink:
        if os.path.exists(LIBS_DIR + depName):
            return False
        else:
            return True
    else:
        if os.path.exists(LIBS_OUTPUT + depName):
            return False
        else:
            return True

    
def needLink(depName):
    if CURRENT_DEP in FORCE_BUILD:
        return True

    if os.path.exists(LIBS_OUTPUT + depName):
        return False
    else:
        return True
    """
    import re

    for l in outlibs:
        rename = None

        if type(l) == list:
            rename = l[1] + getLibExt(l[1])
            l = l[0]

        l += getLibExt(l)
        p, name = os.path.split(l)
        cwd = os.getcwd()
        found = False

        try:
            files = os.listdir(p)
        except:
            return True 

        for f in files:
            if re.match(name + "$", f):
                found = True

        if found is False:
            return True

    return False 
    """

def getLibExt(libName):
    libExt = ".a"

    p, name = os.path.split(libName)

    if ".framework" in name:
        return ""
    else:
        if system == "Windows":
            libExt += ".lib"

    return libExt

def spinningCursor():
    cursor='/-\|'
    i = 0
    while 1:
        yield cursor[i]
        i = (i + 1) % len(cursor)

def hasColours(stream):
    if not hasattr(stream, "isatty"):
        return False
    if not stream.isatty():
        return False # auto color only on TTYs
    try:
        import curses
        curses.setupterm()
        return curses.tigetnum("colors") > 2
    except:
        # guess false in case of error
        return False
class log():
    COLORS = {
        "black": 0,
        "red": 1,
        "green": 2,
        "yellow": 3,
        "blue": 4,
        "magenta": 5,
        "cyan": 6,
        "white": 7
    }
    @staticmethod
    def _clean(string):
        import re
        ansi_escape = re.compile(r'\x1b[^m]*m')
        string = ansi_escape.sub('', string)
        return string.replace("\r", "")

    @staticmethod
    def setOk():
        log.log("\r\x1b[1A[", None, False)
        log.log("✔", log.COLORS["green"], False)
        log.log("]\x1b[1B", None, False);

    @staticmethod
    def setError():
        log.log("\r\x1b[1A[", None, False)
        log.log("✖", log.COLORS["red"], False)
        log.log("]\x1b[1B", None, False)

    @staticmethod
    def spinner(c, state = None):
        log.log("\r[", None, False)
        log.log(c, log.COLORS["cyan"], False)
        log.log("]", None, False)
        if state is not None:
            log.log(state, None, False)

    @staticmethod
    def state(state, color):
        log.log("\r[")
        log.log(state, color)
        log.log("\x1b[0m]")

    @staticmethod
    def error(text, newLine = False):
        log.log("\r[")
        log.log("✖", log.COLORS["red"])
        log.log("]" + text + "\n")

    @staticmethod
    def success(text):
        log.log("\r[")
        log.log("✔", log.COLORS["green"])
        log.log("] " + text + "\n")

    @staticmethod
    def action(text):
        log.state("❖", log.COLORS["yellow"])
        log.log(text + "\n")

    @staticmethod
    def step(text):
        log.state("ᐅ", log.COLORS["cyan"])
        log.log("\x1b[4m" + text + "\x1b[0m\n")

    @staticmethod
    def info(text):
        log.state("ᐅ", log.COLORS["blue"])
        log.log(text + "\n")

    @staticmethod
    def debug(text):
        log.log(text, log.COLORS["cyan"])

    @staticmethod
    def warn(text):
        log.log(text, log.COLORS["yellow"])

    @staticmethod
    def log(text, color=None, logToFile = True):
        import sys
        if hasColours(sys.stdout):
            if color is None:
                seq = "\x1b[0m" + text
            else:
                seq = "\x1b[1;%dm" % (30 + color) + text
            seq += "\x1b[0m"
            print seq,
        else:
            print text,

        if logToFile:
            LOG_FILE.write(log._clean(text))

class spinner():
    STOP_SPINNER_THREAD = False

    @staticmethod
    def start():
        if VERBOSE:
            return

        import threading
        # Workaround for a bug with subprocess.poll() that always return None for long running process
        # Instead the spinner is printed from a thread while communicate() waiting from script ends 
        t = threading.Thread(target=spinner.display)
        t.daemon = True
        t.start()

    @staticmethod
    def stop():
        spinner.STOP_SPINNER_THREAD = True

    @staticmethod
    def display():
        import time

        if sys.stdout.isatty():
            for c in spinningCursor():
                if spinner.STOP_SPINNER_THREAD is True:
                    spinner.STOP_SPINNER_THREAD = False
                    print "\r",
                    sys.stdout.flush()
                    time.sleep(0.2)
                    return

                log.spinner(c)

                sys.stdout.flush()
                time.sleep(0.2)

def runCommand(cmd, **kwargs):
    import sys
    import subprocess

    log.action("Executing " + cmd)

    displaySpinner = True
    stdin = None 

    if "stdin" in kwargs:
        stdin = kwargs["stdin"]

    if "spinner" in kwargs:
        displaySpinner = kwargs["displaySpinner"]

    if VERBOSE:
        displaySpinner = False

    child = None

    if VERBOSE:
        child = subprocess.Popen(cmd, shell=True, stdin=stdin)
    else:
        LOG_FILE.flush()
        child = subprocess.Popen(cmd, shell=True, stdin=stdin, stdout=LOG_FILE, stderr=LOG_FILE)

        if displaySpinner:
            spinner.start()

    output, error = child.communicate()
    code = child.returncode

    if displaySpinner:
        spinner.stop()

    if code != 0:
        if not VERBOSE:
            log.setError()
            print "\r" + output,
            log.info("Error output saved to " + CWD + "/deps-error.log")
            f = open(CWD + '/deps-error.log', 'w')
            f.write(output)
            f.close()
        else:
            log.error("Failed to run previous command")
            sys.exit()
    else:
        log.setOk()
    
    return code, output

def buildDep(depName, directory, buildCommand, **kwargs):
    if "outlibs" in kwargs:
        outlibs = kwargs["outlibs"]
    else:
        outlibs = [directory + "/" + depName]

    if "symlink" in kwargs:
        symlink = kwargs["symlink"]
    else:
        symlink = True

    depName += getLibExt(depName)

    if CURRENT_DEP in FORCE_BUILD:
        build = True
    else:
        build = needBuild(depName, symlink);

    if build:
        import subprocess
        import shutil

        cwd = os.getcwd()
        log.step("Building " + depName)
        os.chdir(directory)

        for cmd in buildCommand:
            import string

            if cmd.startswith("make"):
                cmd += " -j" + str(nbCpu)
            elif cmd.startswith("xcodebuild"):
                cmd += " -jobs " + str(nbCpu)

            code, output = runCommand(cmd)

            if code != 0:
                log.error("Failed to build " + depName)
                os.chdir(cwd)
                sys.exit()

        os.chdir(cwd)
        copyAndLinkDep(outlibs, symlink)
        log.setOk()
    elif not build and needLink(depName):
        copyAndLinkDep(outlibs, symlink)
    elif VERBOSE:
        log.info("Dependency " + depName + " already built and copied. Skipping")

def copyAndLinkDep(outlibs, symlink = True):
    import re
    import shutil
    for l in outlibs:
        rename = None

        if type(l) == list:
            rename = l[1] + getLibExt(l[1])
            l = l[0]

        l += getLibExt(l)
        p, name = os.path.split(l)
        files = os.listdir(p)
        cwd = os.getcwd()
        ok = False

        if VERBOSE:
            log.info("Searching and copying library : " + l)

        for f in files:
            if re.match(name + "$", f):
                ok = True
                if rename is not None:
                    name = rename

                if symlink is True:
                    if os.path.isdir(l):
                        shutil.rmtree(LIBS_DIR + "/" + name, True)
                        shutil.copytree(l, LIBS_DIR + "/" + name)
                    else:
                        shutil.copyfile(l, LIBS_DIR + "/" + name)
                    if system == "Windows":
                        # TODO : Support windows
                        #import win32file
                        #win32file.CreateSymbolicLink(fileSrc, fileTarget, 1)
                        log.error("Not supported")
                        sys.exit()
                    else:
                        os.chdir(LIBS_OUTPUT)

                        try:
                            os.unlink(LIBS_OUTPUT + name)
                        except:
                            pass

                        os.symlink(LIBS_DIR + name, name)

                        os.chdir(cwd)
                else:
                    shutil.copyfile(l, LIBS_OUTPUT + "/" + name)

        if ok == False:
            log.error("Failed to copy and link " + name)
            sys.exit(1);


def downloadAndBuildDeps():
    global CURRENT_DEP

    cwd = os.getcwd()
    os.chdir(THIRD_PARTY)

    # Download everything
    for dep in deps:
        CURRENT_DEP = dep
        if dep not in availableDependencies:
            log.error("Dependency " + dep + " is not available")
            sys.exit()

        dep = availableDependencies[dep]

        if dep["download"] is not None:
            dep["download"]()

    # Build everything
    for dep in deps:
        CURRENT_DEP = dep
        if dep not in availableDependencies:
            log.error("Dependency " + dep + " is not available")
            sys.exit()

        dep = availableDependencies[dep]

        if needDep(dep["name"]):
            if dep["build"] is not None:
                dep["build"]()

    os.chdir(cwd)

def processRequirements(fileName):
    cwd = os.getcwd()
    p, name = os.path.split(fileName)
    if p == "":
        p = "./"

    sys.path.append(os.path.realpath(p))

    os.chdir(p)

    # First, read requirements files
    try:
        with open(name) as f:
            fileDeps = f.read().splitlines()
    except:
        log.error("Failed to open requirements file : " + fileName)
        sys.exit()

    # Look for any python requirements file, and run it
    for dep in fileDeps:
        if dep.endswith(".py"):
                log.info("Importing requirements file \"" + dep + "\" ... ")
                try:
                    tmp = __import__(dep[:-3])
                    tmp.registerDeps()
                except:
                    log.setError()
                    log.error("Failed to load python requirement " + dep)
                    raise
                else:
                    log.setOk()
        elif dep.endswith(".deps"):
            processRequirements(dep)
        else:
            if dep not in deps:
                deps.append(dep)

    os.chdir(cwd)

def buildTarget(project=None, target=None):
    cwd = os.getcwd()
    os.chdir("gyp")

    makeCmd = ""
    if system == "Darwin":
        makeCmd = "xcodebuild -project " + project + ".xcodeproj -jobs " + str(nbCpu)
        if BUILD == "debug":
            makeCmd += " -configuration Debug"

        if target is not None:
            makeCmd += " -target " + target

    elif system == "Linux":
        makeCmd = "CC=" + CLANG + " CXX=" + CLANGPP +" make " + target + " -j" + str(nbCpu)
        if VERBOSE:
            makeCmd += " V=1"
        if BUILD == "debug":
            makeCmd += " BUILDTYPE=Debug"
    else:
        log.error("TODO")
        sys.exit(0)
    
    log.step("Running gyp for target " + target);
    code, output = runCommand(makeCmd)

    if code != 0:
        log.error("Failed to build project")
        sys.exit(1)

    os.chdir(cwd)

    return True

def runGyp(target="all.gyp", args=""):
    cwd = os.getcwd()
    os.chdir("gyp")
    log.step("Preparing gyp")

    code, output = runCommand(GYP + " --include=config.gypi --include=common.gypi --depth ./ %s %s" % (target, args))

    if code != 0:
        sys.exit(1)

    os.chdir(cwd)

    return True

def buildMC():
    # => NSPR
    configure = "./configure"
    nsprDir = os.path.realpath("mozilla-central/nsprpub/")
    if is64bits:
        configure += " --enable-64bit"

    buildDep("libnspr4", "mozilla-central/nsprpub/", [configure, "make"], outlibs=["mozilla-central/nsprpub/dist/lib/libnspr4"])

    nsprFlags = "--with-nspr-cflags=\"-I" + nsprDir + "/dist/include/nspr/\" --with-nspr-libs=\"" + nsprDir + "/dist/lib/libnspr4.a " + nsprDir + "/dist/lib/libplds4.a " + nsprDir + "/dist/lib/libplc4.a\""

    # => mozilla-central
    if BUILD == "debug":
        configure = "./configure --enable-threadsafe --enable-debug --disable-optimize --enable-ctypes " + nsprFlags
        if ENABLE_VALGRIND:
            configure += " --enable-valgrind "
    else:
        configure = "./configure --enable-threadsafe --enable-ctypes " + nsprFlags

    buildDep("libjs_static", "mozilla-central/js/src/", [AUTOCONF, configure, "make"], outlibs=["mozilla-central/js/src/libjs_static"], symlink=False)

def buildGyp():
    global GYP
    if GYP is None:
        GYP = os.path.realpath("./gyp/gyp")

from functools import partial

registerDep("mozilla-central", 
    partial(downloadDep, "mozilla-central", depsURL + "/mozilla-central.bz2", "mozilla-central-.*"), 
    buildMC)

registerDep("c-ares", 
    partial(downloadDep, "c-ares", depsURL + "/c-ares-1.9.1.tar.gz", "c-ares-1.9.1$"),
    partial(buildDep, "libcares", "c-ares", ["./configure", "make"], outlibs=["c-ares/.libs/libcares"]))

registerDep("gyp", 
    partial(downloadDep, "gyp", depsURL + "/gyp.tar.gz"),
    buildGyp)

registerDep("http-parser", 
    None,
    partial(buildDep, "libhttp_parser", "http-parser", ["make package"]))
