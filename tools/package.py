#!/usr/bin/env python

# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

import sys, os, imp
import shutil

from konstructor import Konstruct
from konstructor import Builder
from konstructor import Platform
from konstructor import Utils
from konstructor import Log
from konstructor import Variables
from konstructor import CommandLine
from konstructor import Deps

Gyp = Builder.Gyp

Konstruct.setConfigs(["release"])
Gyp.setConfiguration("Release")
Variables.set("verbose", True)

Gyp.set("native_enable_breakpad", 0)

OUTPUT_BINARY = None
SIGN_IDENTITY = None
FLAVOUR = None

LICENSE = """Nidium is released under MIT License. This software is provided as is without warranty of any kind.

This Software includes third-party libraries released with various open source license. Please refer to https://github.com/nidium/Nidium/blob/master/LICENSE for more information.
"""

@CommandLine.option("--server", help="Package Nidium Server", default=False)
def server(server):
    global FLAVOUR, OUTPUT_BINARY

    if not server:
        return

    FLAVOUR = "server"
    OUTPUT_BINARY = "bin/nidium-server"

@CommandLine.option("--frontend", help="Package Nidium Frontend", default=False)
def frontend(frontend):
    global FLAVOUR, OUTPUT_BINARY

    if not frontend:
        return

    FLAVOUR = "frontend"

    if Platform.system == "Darwin":
        OUTPUT_BINARY = "bin/nidium.app/Contents/MacOS/nidium"
    elif Platform.system == "Linux":
        OUTPUT_BINARY = "bin/nidium"

@CommandLine.option("--sign", help="Sign .dmg file with the identity providen")
def sign(sign):
    if not sign:
        return

    SIGN_IDENTITY = sign

def signCode(path):
    if SIGN_IDENTITY is None:
        Log.info("No identity providen, not signing app")
        return

    Log.info("Signing nidium package...")

    code, output = Utils.run(" ".join([
        "codesign",
        "--force",
        "--sign",
        "'Developer ID Application: %s '" % SIGN_IDENTITY,
        path
    ]), failExit=False)

    if code != 0:
        Log.error("WARNING : App signing failed with identity %s. Not signing app" % identity)
        Log.error(output)

    Log.info(path)

def stripExecutable():
    Log.info("Striping executable")
    Utils.run("strip " + OUTPUT_BINARY)

def getPackageName():
    import time
    import subprocess

    revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
    tag = None
    arch = ""
    name = ""

    try:
        tag = subprocess.check_output(["git", "describe",  "--exact-match", revision], stderr=subprocess.PIPE)
        print "Tag is " + tag
    except:
        tag = None

    if Platform.wordSize == 64:
        arch = "x86-64"
    else:
        arch = "i386"

    if tag is None:
        datetime = time.strftime("%Y%m%d_%H%M%S")
        name = "Nidium_%s_%s_%s_%s_%s" % (FLAVOUR, datetime, revision, Platform.system, arch)
    else:
        name = "Nidium_%s_%s_%s_%s" % (FLAVOUR, tag, Platform.system, arch)

    return name

def packageServer():
    name = getPackageName()

    tmpDir = os.path.join("build", "package", "nidium-server")

    Utils.mkdir(tmpDir)

    Utils.run("echo \"%s\" > %s/LICENSE" % (LICENSE, tmpDir))

    shutil.copy(OUTPUT_BINARY, tmpDir)

    with Utils.Chdir("build/package/"):
        Utils.run("tar -czvf %s.tar.gz nidium-server/" % (name))

    shutil.rmtree(tmpDir);

def packageFrontend():
    import tarfile 

    path = "bin/"
    baseResources = "resources/"

    name = getPackageName()

    tmpDir = os.path.join("build", "package", "nidium.tmp")
    Utils.mkdir(tmpDir)

    Utils.run("echo \"%s\" > %s/LICENSE" % (LICENSE, tmpDir))

    if Platform.system == "Darwin":
        signCode(path + "nidium.app")

        Log.info("Create dmg...")

        resources = "%s/osx/" % (baseResources)
        name += ".dmg"
        cmd = [
            "tools/installer/osx/create-dmg",
            "--volname 'Nidium'",
            "--no-internet-enable",
            "--volicon " + resources + "/nidium.icns",
            "--background " + resources + "/dmg-background.png",
            "--window-size 555 418",
            "--icon-size 96",
            "--eula %s/LICENSE" % tmpDir,
            "--app-drop-link 460 290",
            "--icon 'nidium.app' 460 80",
            "build/package/%s" % name,
            path + "nidium.app/"
        ]
        code, output = Utils.run(" ".join(cmd))
        if code != 0:
            Utils.exit("Failed to build dmg")
    elif Platform.system == "Linux":
        resources = "%s/linux/" % (baseResources)
        name += ".run"

        Utils.mkdir(tmpDir + "/dist/")
        Utils.mkdir(tmpDir + "/resources/")

        shutil.copy(resources + "/nidium.desktop", tmpDir + "/resources/")
        shutil.copy(resources + "/x-application-nidium.xml", tmpDir + "/resources/")
        shutil.copy(baseResources + "/icons/nidium.iconset/nidium_32x32@2x.png", tmpDir + "/resources/nidium.png")
        shutil.copy(resources + "/installer.sh", tmpDir)
        shutil.copy(path + "nidium",  tmpDir + "/dist/")

        # XXX : Uncomment once breakpad & landing net are back
        #shutil.copy(path + "nidium-crash-reporter", tmpDir + "dist/")

        Utils.run("tools/installer/linux/makeself.sh --license %s/LICENSE %s build/package/%s 'Nidium installer' ./installer.sh " % (tmpDir, tmpDir, name))
    else:
        # Window TODO
        print("TODO")

    shutil.rmtree(tmpDir);

def package():
    stripExecutable()

    Log.info("Packaging %s" % FLAVOUR)

    if FLAVOUR == "frontend":
        packageFrontend()
    elif FLAVOUR == "server":
        packageServer()

if __name__ == '__main__':
    # First, parse command line arguments, so we 
    # can print the help message (if needed) and 
    # exit before doing anything else
    CommandLine.parse()

    if FLAVOUR is None:
        Utils.exit("Please specify a flavour to package with --frontend or --server");

    if FLAVOUR == "frontend":
        configure = imp.load_source("configure", "configure_frontend");

        # Make sure we are using the correct deps for the 
        # current configuration before building dir2nvfs
        Deps._process()

        # Build dir2nvfs and the embed file
        configure.addEmbed()
    else:
        imp.load_source("configure", "configure_server");

    Konstruct.start() 

    package()
