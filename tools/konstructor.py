import sys,os

from pprint import pprint

LOG_FILE = open('./konstruct.log', 'w')
OUTPUT = "build/"
ROOT = os.getcwd()

# {{{ Variables
class Variables:
    _store = {}

    @staticmethod
    def set(key, value):
        Variables._store[key] = value

    @staticmethod
    def get(key, default=None):
        if key in Variables._store:
            return Variables._store[key]
        else:
            return default
# }}}

# {{{ Konstruct
class Konstruct:
    _configuration = ["default"]

    @staticmethod
    def start():
        CommandLine.parse()
        Deps._process()
        Build.run()
        if len(Platform._exported)  > 0:
            Log.info("\n\n--------------------------\nYou have enabled some features that needs to export shell variables. Please execute the following commands :\n")
            for cmd in Platform._exported:
                Log.info(cmd)
            Log.info("--------------------------")


    @staticmethod
    def config(*args):
        for config in args:
            if config in Konstruct._configuration:
                return True

    @staticmethod
    def setConfigs(configs):
        for config in configs:
            if config not in Konstruct._configuration:
                Konstruct._configuration.append(config)

    @staticmethod
    def getConfigs():
        return Konstruct._configuration
# }}}

# {{{ ComandLine
from collections import OrderedDict
from optparse import OptionParser
import types
class CommandLine:
    optionParser = OptionParser(usage="Usage: %prog [options]")
    _options = OrderedDict()
    _required = []

    @staticmethod
    def parse():
        options, args = CommandLine.optionParser.parse_args()
        optionsDict = vars(options)

        for required in CommandLine._required:
            if not optionsDict.get(required):
                CommandLine.optionParser.error("You need to specify %s argument" % required)

        out = {}
        for name, callbacks in CommandLine._options.items():
            for callback in callbacks:
                if callback not in out:
                    out[callback] = []

                for option, value in vars(options).iteritems():
                    if option == name:
                        out[callback].append(value)


        for callback, args in out.items():
            callback(*args)

    @staticmethod
    def option(name, **kwargs):
        # Add the option now, so we can keep the order of options
        CommandLine._options[name] = []
        def decorator(f):
            default = None
            action = "store"
            t = "string"

            if "required" in kwargs:
                CommandLine._required.append(name)

            if "default" in kwargs:
                default = kwargs["default"]
                if type(default) == types.BooleanType:
                    t = None
                    if default is True:
                        action = "store_false"
                    else:
                        action = "store_true"
                elif type(default) == types.IntType:
                    t = "int"

            CommandLine._options[name].append(f)
            CommandLine.optionParser.add_option(name, dest=name, default=default, action=action, type=t)

            return f

        return decorator 

@CommandLine.option("--configuration", default="")
def configuration(config):
    config = config.split(",")
    if len(config) > 0:
        Konstruct.setConfigs(config)

@CommandLine.option("--verbose", default=False)
def verbose(verbose):
    Variables.set("verbose", True)

@CommandLine.option("--force-download", default="")
@CommandLine.option("--force-build", default="")
@CommandLine.option("--force", default="")
def forceDownload(forceDownload, forceBuild, force):
    if forceDownload:
        forceDownload = forceDownload.split(",")
    else:
        forceDownload = []

    if forceBuild:
        forceBuild = forceBuild.split(",")
    else:
        forceBuild = []

    if force:
        force = force.split(",")
        forceDownload += force
        forceBuild += force

    for d in forceDownload + forceBuild:
        if d in AVAILABLE_DEPS["default"]:
            if d in forceDownload:
                Log.info("Forcing download for %s" % d)
                AVAILABLE_DEPS["default"][d].needDownload = True
            if d in forceBuild:
                Log.info("Forcing download for %s" % d)
                AVAILABLE_DEPS["default"][d].needBuild = True
        else:
            Log.warn("Can't force download or build for %s. Dependency not found" % d)
# }}}

# {{{ Platform
import platform
import multiprocessing

class Platform:
    system = platform.system()
    cpuCount = multiprocessing.cpu_count()
    wordSize = 64 if sys.maxsize > 2**32 else 32 
    _exported = []

    @staticmethod
    def exportEnviron(arg):
        Platform._exported.append("export " + arg)

    @staticmethod
    def setEnviron(*args):
        for env in args:
            tmp = env.split("=", 1)
            if tmp[0].endswith("+"):
                key = tmp[0][:-1]
                if key in os.environ:
                    os.environ[key] = os.environ.get(key, "") + os.pathsep + tmp[1]
                else:
                    os.environ[key] = tmp[1]
            else:
                os.environ[tmp[0]] = tmp[1]

# }}}

# {{{ ConfigCache
class ConfigCache:
    def __init__(self, f):
        self.file = f;

    def update(self, key, entry):
        configCache = ConfigCache._read(self.file)
        eHash = entry["hash"]
        eConfig = entry["config"]

        if configCache is None or key not in configCache:
            # entry does not exists in cache
            if configCache is None:
                configCache = {key: {eHash: eConfig}}
            else:
                configCache[key] = {eHash: eConfig}

            ConfigCache._write(configCache, self.file)
        else:
            if eHash not in configCache[key]:
                # Cache for the data is new (different config)

                # We can only have one cache entry per config per dep
                # find and remove duplicate
                for h in configCache[key].keys():
                    cachedConf = configCache[key][h]
                    if h != eHash and cachedConf == eConfig:
                        del configCache[key][h]

                configCache[key][eHash] = eConfig 
                ConfigCache._write(configCache, self.file)
            else:
                # Entry already exists
                # Nothing to save
                pass

    def find(self, key, data):
        configCache = ConfigCache._read(self.file)
        newCacheHash = ConfigCache._generateHash(data)
        config = "-".join(Konstruct.getConfigs()).rstrip("-")
        ret = {"new": True, "hash":  newCacheHash, "config": config}

        if configCache is not None and key in configCache:
            if newCacheHash in configCache[key]:
                # data is already found in cache but for another configuration
                # We just return the previously found matching config
                ret["new"] = False
                ret["config"] = configCache[key][newCacheHash]

        return ret

    @staticmethod
    def _generateHash(data):
        import pickle
        import hashlib
        return hashlib.md5(pickle.dumps(data)).hexdigest()

    @staticmethod
    def _write(stamps, dst):
        import json
        open(dst, "w").write(json.dumps(stamps, indent=4))
        
    @staticmethod
    def _read(location):
        import json
        try:
            return json.loads(open(location, "r").read())
        except:
            return None
# }}}

# {{{ Utils
class Utils:
    class Chdir:
        def __init__(self, dir):
            self.cwd = os.getcwd()
            self.dir = dir

        def __enter__(self):
            os.chdir(self.dir)

        def __exit__(self, type, value, traceback):
            os.chdir(self.cwd)

    @staticmethod
    def patch(directory, patchFile, pNum=1):
        if not os.path.exists(directory):
            Utils.exit("Directory %s does not exist. Not patching." % directory)

        if not os.path.exists(patchFile):
            Utils.exit("Patch file %s does not exist. Not patching." % patchFile)

        import subprocess

        pNum = "-p" + str(pNum);
        patch = open(patchFile)
        nullout = open(os.devnull, 'w')

        with Utils.Chdir(directory):
            # First check if the patch might have been already aplied
            applied = subprocess.call(["patch", pNum, "-N", "-R", "--dry-run", "--silent"], stdin=patch, stdout=nullout, stderr=subprocess.STDOUT)
            
            if applied == 0:
                Log.info("    Already applied patch "+ patchFile + " in " + directory + ". Skipping.")
            else:
                Log.info("    Applying patch " + patchFile)

                # Check if the patch will succeed
                patch.seek(0)
                patched = subprocess.call(["patch", pNum, "-N", "--dry-run", "--silent"], stdin=patch, stderr=subprocess.STDOUT)
                if patched == 0:
                    patch.seek(0)
                    success, output = Utils.run("patch " + pNum + " -N", stdin=patch)
                    if success != 0:
                        Utils.exit("Failed to patch")
                else:
                    Utils.exit("Failed to patch")

            patch.close()
            nullout.close()

    @staticmethod
    def symlink(src, dst):
        if os.path.lexists(dst):
            try:
                os.unlink(dst)
            except:
                Utils.exit("Can not unlink %s/%s. Manually rename or remove this file" % (os.getcwd(), dst))

        if Platform.system == "Windows":
            # TODO : Not supported
            #import win32file
            #win32file.CreateSymbolicLink(fileSrc, fileTarget, 1)
            Logs.error("no windows support for symlink")
            Utils.exit()
        else:
            os.symlink(src, dst)

    @staticmethod
    def mkdir(path):
        import errno
        try:
            os.makedirs(path)
        except OSError as exc: # Python >2.5
            if exc.errno == errno.EEXIST and os.path.isdir(path):
                pass
            else: raise

    @staticmethod
    def exit(reason = None):
        if reason:
            Log.echo(reason)
        sys.exit()

    @staticmethod
    def run(cmd, **kwargs):
        import subprocess

        Log.echo("Executing " + cmd)

        displaySpinner = True
        stdin = None 
        failExit = True

        if "stdin" in kwargs:
            stdin = kwargs["stdin"]

        if "failExit" in kwargs:
            failExit = kwargs["failExit"]

        child = None

        if Variables.get("verbose", False):
            child = subprocess.Popen(cmd, shell=True, stdin=stdin)
        else:
            LOG_FILE.flush()
            child = subprocess.Popen(cmd, shell=True, stdin=stdin, stdout=LOG_FILE, stderr=LOG_FILE)

        output, error = child.communicate()
        code = child.returncode

        if code != 0:
            if failExit:
                Utils.exit("Failed to run previous command")
        else:
            Log.success("Success")
        
        return code, output

    @staticmethod
    def extract(path, destination=None):
        import shutil

        if os.path.isdir(path):
            return

        extension = os.path.splitext(path)[1]
        if extension == ".zip":
            from zipfile import ZipFile
            zip = ZipFile(path)
            zip.extractall(destination)
            zip.close()
        elif extension in [".tar", ".gz", ".bz2", ".bzip2", ".tgz"]:
            import tarfile
            tar = tarfile.open(path)
            tar.extractall(destination)
            tar.close()
        else:
            # Single file downloaded, not an archive
            # Nothin to do
            Log.info("Nothing to extract for " + path)
            return

        with Utils.Chdir(destination):
            # Detect if the archive have been extracted to a subdirectory of destination
            # if it's the case, move the content of the directory to the parent dir
            files = os.listdir(".")
            if len(files) == 1 and os.path.isdir(files[0]):
                for f in os.listdir(os.path.join(".", files[0])):
                    shutil.move(os.path.join(files[0], f), ".")
                os.rmdir(files[0])

    @staticmethod
    def download(location, downloadDir=None, destinationDir=None):
        import types

        if downloadDir is None:
            downloadDir = tempfile.gettempdir()

        if type(location) != types.StringType:
            return location.download(destinationDir)
        else:
            if location.startswith("http"):
                return Utils._httpDownload(location, downloadDir, destinationDir)
            else:
                Utils.exit("Protocol not supported for downloading " + location)
        
    @staticmethod
    def _httpDownload(url, downloadDir, destinationDir):
        import urllib2
        import tempfile

        file_name = url.split('/')[-1]
        u = urllib2.urlopen(url)
        f = open(os.path.join(downloadDir, file_name), "wb")
        meta = u.info()
        file_size = int(meta.getheaders("Content-Length")[0])

        file_size_dl = 0
        block_sz = 8192
        while True:
            buff = u.read(block_sz)
            if not buff:
                break

            file_size_dl += len(buff)
            f.write(buff)

        f.close()

        print("dest=%s" % destinationDir)
        if destinationDir:
            print("extract")
            Utils.extract(os.path.join(downloadDir, f.name), destinationDir)
# }}}

# {{{ Logs
class Log:
    @staticmethod
    def echo(string):
        print string

    @staticmethod
    def info(string):
        Log.echo(string)

    @staticmethod
    def debug(string):
        Log.echo("[DEBUG] " + string)

    @staticmethod
    def warn(string):
        Log.echo(string)

    @staticmethod
    def success(string):
        Log.echo(string)

    @staticmethod
    def error(string):
        Log.echo(string);
# }}}

# {{{ Deps
# TODO : Cleanup symlink handling (build)
from collections import OrderedDict
AVAILABLE_DEPS = {"default":{}}
DEPS = OrderedDict()
class Dep:
    def __init__(self, name, fun, options={}):
        self.function = fun
        self.options = options
        self.name = name

        self.needDownload = False
        self.needBuild = False

        self.linkDir = None
        self.outputFiles = []
        self.extractDir = None
        self.outputsDir = None

        self.downloadConfig = None
        self.buildConfig = None
        self.cache = ConfigCache("konstruct.cache")

    def prepare(self):
        Log.debug("Preparing " + self.name)
        if self.function is not None:
            options = self.function()
            if options is not None:
                # merge decoration options with the options 
                # returned by the decorated function
                self.options = dict(self.options.items() + options.items())

        # Check if we need to download the dep
        if "location" in self.options:
            cache = self.cache.find(self.name + "-download", self.options["location"])
            self.downloadConfig = cache

            self.linkDir = {"src": os.path.join("." + cache["config"], self.name), "dest": self.name}
            self.extractDir = self.linkDir["src"]

            exists = os.path.exists(self.extractDir)
            if cache["new"] and not exists:
                Log.debug("Need download because configuration for this dep is new")
                self.needDownload = True
            elif not exists:
                Log.debug("Need download because output dir does not exists")
                self.needDownload = True
            elif not os.path.islink(self.linkDir["dest"]):
                Utils.symlink(self.linkDir["src"], self.linkDir["dest"])

        # Define some variables needed for building/symlinking
        cache = self.cache.find(self.name + "-build", self.options)
        self.buildConfig = cache
        self.outputsDir = os.path.join(ROOT, OUTPUT, "third-party", "." + cache["config"])
        Utils.mkdir(self.outputsDir)

        if self.needDownload:
            self.needBuild = True
        elif not self.needBuild:
            if cache["new"]:
                Log.debug("Need build, because configuration for this dep is new")
                self.needBuild = True
            elif "outputs" in self.options:
                # If we don't have any configuration change, make sure that the outputs 
                # exists and are more recent than the downloaded/extracted directory

                # Get the time of the directory
                srcDir = self._getDir();
                try:
                    dirTime = os.path.getmtime(os.path.realpath(srcDir))
                except:
                    self.needBuild = True
                    return

                for output in self.findOutputs():
                    if output["found"] is False:
                        Log.debug("Need build %s, because output file %s havn't been found" % (self.name, output["src"]))
                        self.needBuild = True
                        break
                    # This code have some issues 
                    # - It does not detect change made in subdirectories
                    # - Changing configuration might trigger a rebuild 
                    #   since the outputs could be older than the directory when depdency is rebuilt in another configuration
                    #
                    # Until a better alternative is found, do not check if output is more recent
                    """
                    else:
                        import datetime
                        try:
                            outFileTime = os.path.getmtime(os.path.join(self.outputsDir, output["file"]))
                        except:
                            # Symlink does not exist, it will be created back at link time
                            continue

                        if dirTime is None:
                            # Make sure dest file is more recent than src file
                            if outFileTime > os.path.getmtime(output["src"]):
                                Log.debug("Need build, because dep (%s) file %s is more recent than ouput file %s " % (self.name, output["src"], output["file"]))
                                self.needBuild = True
                        elif dirTime > outFileTime:
                            Log.debug("Need build, because dep (%s) is more recent than ouput file %s (%s / %s)" % (srcDir, output["file"], 
                                datetime.datetime.fromtimestamp(dirTime).strftime('%Y-%m-%d %H:%M:%S'), 
                                datetime.datetime.fromtimestamp(outFileTime).strftime('%Y-%m-%d %H:%M:%S'),
                            ))
                            self.needBuild = True
                            break
                    """

    def download(self):
        if not self.needDownload:
            return 

        Log.info("Downloading %s" % self.name)
        Utils.download(self.options["location"], downloadDir=".", destinationDir=self.extractDir)

        if self.linkDir:
            # Make the dep directory point to the directory matching the configuration
            Utils.symlink(self.linkDir["src"], self.linkDir["dest"])

        self.cache.update(self.name + "-download", self.downloadConfig);

    def patch(self):
        if "patchs" not in self.options:
            return

        Log.debug("Patching " + self.name)
        for p in self.options["patchs"]:
            Utils.patch(self.name, p)

    def _getDir(self):
        newDir = "." 
        if "location" in self.options:
            newDir = self.name
        if "chdir" in self.options:
            newDir = os.path.join(newDir, self.options["chdir"])

        return newDir

    def build(self):
        newDir = self._getDir()

        if not os.path.exists(newDir):
            Utils.mkdir(newDir)

        with Utils.Chdir(newDir):
            if self.needBuild and "build" in self.options:
                Log.info("Bulding " + self.name)
                for cmd in self.options["build"]:
                    if hasattr(cmd, '__call__'):
                        cmd()
                    elif cmd.startswith("make"):
                        cmd += " -j" + str(Platform.cpuCount)
                    elif cmd.startswith("xcodebuild"):
                        cmd += " -jobs " + str(Platform.cpuCount)
                    Utils.run(cmd)

            self.symlinkOutput()

        self.cache.update(self.name + "-build", self.buildConfig)

    def findOutputs(self):
        import re

        outputs = []

        if "outputs" not in self.options:
            return outputs

        depDir = os.path.join(Deps.path, self._getDir())
        with Utils.Chdir(depDir):
            for output in self.options["outputs"]:
                rename = None
                found = False
                copy = False
                if type(output) == list:
                    rename = output[1]
                    outFile = output[0]
                elif type(output) == dict:
                    copy = True
                    outFile = output["src"]
                else:
                    outFile = output

                path, name = os.path.split(outFile)
                if path == "":
                    path = "."

                out = {"copyOnly": False, "found": False, "src": os.path.join(depDir, path, outFile)}

                try:
                    files = os.listdir(path) 
                except:
                    outputs.append(out)
                    continue
                    
                for f in files:
                    if re.match(name, f):
                        out["found"] = True
                        if rename:
                            out["file"] = re.sub(name, rename, f) 
                        elif copy:
                            out["copyOnly"] = True
                            out["file"] = os.path.join("..", output["dest"])
                        else:
                            out["file"] = f

                        out["src"] = os.path.join(depDir, path, f)
                        break

                outputs.append(out)

        return outputs

    def symlinkOutput(self):
        import shutil
        import re

        if "outputs" not in self.options:
            Log.debug("No outputs for " + self.name)
            return

        outputs = self.findOutputs()
        for output in outputs:
            if output["found"]:
                destDir = os.path.join(Deps.getDir(), "..", OUTPUT, "third-party", "." + self.buildConfig["config"])
                destFile = os.path.join(destDir, output["file"])
                Utils.mkdir(destDir)

                if self.needBuild or not os.path.exists(destFile) :
                    # New outputs have been generated
                    # Copy them to the build dir 
                    Log.debug("Found output %s, copy to %s" % (output["src"], destFile))
                    shutil.copyfile(output["src"], destFile)

                # Symlink the current config
                if not output["copyOnly"]:
                    Log.debug("symlink src=%s dst=%s" % (destFile, os.path.join(self.outputsDir, "..", output["file"])))
                    Utils.symlink(destFile, os.path.join(self.outputsDir, "..", output["file"]))
            else:
                Utils.exit("Output %s for %s not found" % (output["src"], self.name))

class Deps:
    path = os.path.abspath("third-party")
    deps = []

    class Konstruct:
        def __init__(self, name, location):
            self.location = location
            self.name = name

        def importDep(self):
            import imp
            Log.debug("Importing konstruct dependency " + self.name)

            path, file = os.path.split(self.location)
            # Since dependency can be nested in directories we need to chdir
            with Utils.Chdir(path):
                try:
                    imp.load_source(self.name, file)
                except Exception as e:
                    Utils.exit("Failed to import konstruct dependency %s : %s" % (self.location, e))
            
    class Repo:
        def __init__(self, location, revision=None):
            self.location = location
            self.revision = str(revision)

    class SvnRepo(Repo):
        def download(self, destination):
            if not os.path.isdir(destination):
                Utils.run("svn checkout %s %s" % (self.location, destination))

            with Utils.Chdir(destination):
                if self.revision:
                    Utils.run("svn up -r" + self.revision)

    class Gclient():
        _exec = "gclient"

        @staticmethod
        def setExec(path):
            Deps.Gclient._exec = os.path.abspath(path)

        def __init__(self, location, revision=None):
            self.location = location
            self.revision = revision

        def download(self, destination):
            Utils.run("%s config --name %s --unmanaged %s" % (Deps.Gclient._exec, destination, self.location))
            Utils.run("%s sync %s" % (Deps.Gclient._exec, "--revision=" + self.revision if self.revision else ""))


    class GitRepo:
        def __init__(self, location, revision=None, branch=None, tag=None):
            self.location = location
            self.revision = revision
            self.branch = branch
            self.tag = tag

        def download(self, destination):
            if not os.path.isdir(destination):
                Utils.run("git clone %s %s" % (self.location, destination))

            with Utils.Chdir(destination):
                if self.tag:
                    Utils.run("git checkout tags/" + self.tag)
                if self.branch:
                    Utils.run("git checkout " + self.branch)

                if self.revision:
                    Utils.run("git checkout " + self.revision)

    @staticmethod
    def _process():
        Utils.mkdir(Deps.path)
        Utils.mkdir(os.path.join(OUTPUT, "third-party"))

        for name in Deps.deps:
            """
            for config in Konstruct.getConfigs():
                if name in AVAILABLE_DEPS[config]:
                    DEPS[name] = AVAILABLE_DEPS[config][name]
                    break
            """


            if name in AVAILABLE_DEPS["default"]:
                DEPS[name] = AVAILABLE_DEPS["default"][name]
            else:
                Utils.exit("Dependency %s is not available" % name)

        """
        if name not in DEPS:
            if Konstruct.config("default") and name in AVAILABLE_DEPS["default"]:
                DEPS[name] = AVAILABLE_DEPS["default"][name]
            else:
                Utils.exit("Dependency %s is not available" % name)
        """

        with Utils.Chdir(Deps.path):
            for name, d in DEPS.items():
                d.prepare()
                d.download()

            for name, d in DEPS.items():
                d.patch()
                d.build()

    @staticmethod
    def register(name, **kwargs):
        def decorator(f):
            d = Dep(name, f, kwargs)

            configuration = "default"

            if configuration not in AVAILABLE_DEPS:
                AVAILABLE_DEPS[configuration] = {}

            AVAILABLE_DEPS[configuration][name] = d
            """
            if "configuration" in kwargs:
                configuration = kwargs["configuration"]

                if type(configuration) == list:
                    for config in configuration:
                        if config not in AVAILABLE_DEPS:
                            AVAILABLE_DEPS[config] = {}

                        AVAILABLE_DEPS[config][name] = d
                else:
                    if configuration not in AVAILABLE_DEPS:
                        AVAILABLE_DEPS[configuration] = {}

                    AVAILABLE_DEPS[configuration][name] = d
            """

        return decorator 

    @staticmethod
    def setDir(path):
        Deps.path = os.path.abspath(path);

    @staticmethod
    def getDir():
        return Deps.path
        
    @staticmethod
    def set(*args):
        for dep in args:
            if isinstance(dep, Deps.Konstruct):
                dep.importDep()
            else:
                Deps.deps.append(dep)
# }}}

# {{{ Builders
BUILD = []
class Build:
    @staticmethod
    def add(builder):
        BUILD.append(builder)

    @staticmethod
    def run():
        for b in BUILD:
            b.run()

class Builder:
    class Gyp:
        _args = ""
        _config = None
        _defines = {}
        _exec = None

        @staticmethod
        def setArgs(args):
            Builder.Gyp._args = args;

        @staticmethod
        def set(key, value):
            Builder.Gyp._defines[key] = value

        @staticmethod
        def setExec(path):
            Builder.Gyp._exec = path;

        @staticmethod
        def setConfiguration(config):
            Builder.Gyp._config = config;

        def __init__(self, name):
            self.name = name

        def run(self, target=None, parallel=True):
            defines = ""
            for key, value in Builder.Gyp._defines.iteritems():
                defines += " -D%s=%s" % (key, value)
            defines += " "

            code, output = Utils.run("%s --generator-output=%s %s %s %s" % (Builder.Gyp._exec, "build", defines, Builder.Gyp._args, self.name))
            cwd = os.getcwd()

            os.chdir(OUTPUT)

            runCmd = ""

            if Platform.system == "Darwin":
                project = os.path.splitext(self.name)[0]
                runCmd = "xcodebuild -project " + project + ".xcodeproj"
                if parallel:
                    runCmd += " -jobs " + str(Platform.cpuCount)
                if Builder.Gyp._config is not None:
                    runCmd += " -configuration " + Builder.Gyp._config

                if target is not None:
                    runCmd += " -target " + target

            elif Platform.system == "Linux":
                #runCmd = "CC=" + CLANG + " CXX=" + CLANGPP +" make " + target + " -j" + str(nbCpu)
                runCmd = "make"

                if target is not None:
                    runCmd += " " + target
                if Variables.get("verbose", False):
                    runCmd += " V=1"
                if Builder.Gyp._config is not None:
                    runCmd += " BUILDTYPE=" + Builder.Gyp._config
                if parallel:
                    runCmd += " -j%i" % Platform.cpuCount
            else:
                # TODO : Windows support
                Utils.exit("Missing windows support");
            
            Log.debug("Running gyp. File=%s Target=%s" % (self.name, target));

            code, output = Utils.run(runCmd)

            if code != 0:
                Utils.exit("Failed to build project")

            os.chdir(cwd)

            return True
# }}}
