from functools import partial
import deps

def buildLevelDB():
    flags = ""
    if deps.system == "Darwin":
        flags = "CXXFLAGS='-stdlib=libc++ -mmacosx-version-min=10.7' CFLAGS='-mmacosx-version-min=10.7'"

    deps.buildDep("libleveldb", "leveldb", [flags + " make"], outlibs=["leveldb/libleveldb"])

def buildOpenSSL():
    configure = "./config"
    if deps.system == "Darwin":
        configure = "./Configure darwin64-x86_64-cc -no-shared"

    deps.buildDep("libssl", "openssl", [configure, "make build_crypto", "make build_ssl"], outlibs=["openssl/libssl", "openssl/libcrypto"])

def registerDeps():
    deps.registerDep("leveldb",
        partial(deps.downloadDep, "leveldb", deps.depsURL + "/leveldb.tar.gz"),
        buildLevelDB)

    deps.registerDep("openssl",
        partial(deps.downloadDep, "openssl", deps.depsURL + "/openssl-1.0.1g.tar.gz", "openssl"),
        buildOpenSSL)
