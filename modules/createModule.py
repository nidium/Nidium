#!/usr/bin/python

import click, os, stat

def createConfigure(name):
    content = """#!/usr/bin/python
import sys 
import os

from konstructor import Deps
from konstructor import CommandLine
from konstructor import Build
from konstructor import Builder
from konstructor import Konstruct

Gyp = Builder.Gyp

if __name__ == '__main__':
    path = '../../'

    Deps.set(
        Deps.Konstruct("nativejscore", path + "nativejscore/configure")
    )

    CommandLine.parse()

    Gyp.setArgs('--depth=./ --include=%s/gyp/config.gypi --include=%s/gyp/common.gypi' % (path, path))
    Build.add(Gyp('gyp/{0}.gyp'));

    Konstruct.start()"""

    with open("%s/configure" % name, "w") as configure:
        configure.write(content.format(name))

    st = os.stat("%s/configure" % name)
    os.chmod("%s/configure" % name, st.st_mode | stat.S_IEXEC)

def createGyp(name, classname):
    content = """{{
    'targets': [{{
        'target_name': '{0}',
        'type': 'shared_library',
        'dependencies': [
            '<(native_nativejscore_path)/gyp/nativejscore.gyp:nativejscore-includes', 
            '<(native_network_path)/gyp/network.gyp:nativenetwork-includes'
        ],
        'include_dirs': [
            '<(third_party_path)/'
        ],
        'sources': [ '../{1}.cpp'],
        'conditions': [
            ['OS=="mac"', {{
                'xcode_settings': {{
                    'OTHER_LDFLAGS': [
                        '-undefined suppress',
                        '-flat_namespace'
                    ],
                }},
            }},{{
                'cflags': [
                    '-fPIC',
                ],
                'ldflags': [
                    '-fPIC',
                ],
            }}]
        ],
    }}],
}}"""

    with open("%s/gyp/%s.gyp" % (name, name), "w") as gypfile:
        gypfile.write(content.format(name, classname))

def createSource(name, classname):
    with open('templateSource.txt', 'r') as srcfile:
        src = srcfile.read()

    with open("%s/%s.cpp" % (name, classname), "w") as cppfile:
        cppfile.write(src.format(classname=classname))

    with open('templateSourceHeader.txt', 'r') as srcfile:
        src = srcfile.read()

    with open("%s/%s.h" % (name, classname), "w") as cppfile:
        cppfile.write(src.format(classname=classname))


def createMakefile(name, classname):
    content = """all: lockfile

lockfile: {0}.cpp {0}.h
\t@touch lockfile
\tPYTHONPATH=../../ ./configure --debug --third-party=../../third-party/"""

    with open("%s/Makefile" % name, "w") as makefile:
        makefile.write(content.format(classname))

@click.command()
@click.option('--name', prompt='The module name',
              help='What\'s the module name.')
@click.option('--classname')
def createmodule(name, classname):
    """Create a new Native Module."""

    if classname is None:
        classname = name
    try:
        os.makedirs(name + "/gyp")
    except:
        pass

    createConfigure(name)
    createGyp(name, classname)
    createSource(name, classname)
    createMakefile(name, classname)

    print("Now build with cd {0} && make".format(name))

if __name__ == '__main__':
    createmodule()