import deps

def build():
    deps.runGyp("all.gyp", deps.gypArgs)
    deps.buildTarget("all", "native-server")

def registerDeps():
    pass

def noop(more):
    pass

deps.registerAction(noop, None, pre=build)
