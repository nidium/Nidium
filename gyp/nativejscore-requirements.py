from functools import partial
import deps

def buildLevelDB():
    flags = ""
    if deps.system == "Darwin":
        flags = "CXXFLAGS='-stdlib=libc++ -mmacosx-version-min=10.7' CFLAGS='-mmacosx-version-min=10.7'"

    deps.buildDep("libleveldb", "leveldb", [flags + " make"], outlibs=["leveldb/libleveldb"])

def registerDeps():
    deps.registerDep("leveldb",
        partial(deps.downloadDep, "leveldb", deps.depsURL + "/leveldb.tar.gz"),
        buildLevelDB)
