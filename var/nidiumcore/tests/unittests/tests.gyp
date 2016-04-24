# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'nidiumcore-unittests',
        'product_dir': '<(native_tests_output)',
        'type': 'executable',
        'dependencies': [
            '../../../../network/gyp/network-unittests.gyp:unittests-settings',
            '../../../../gyp/nidiumcore.gyp:*',
            '../../../../network/gyp/network.gyp:*',
        ],
        'sources': [
            'unittest.cpp',
            'args.cpp',
            'db.cpp',
            'events.cpp',           #dummy
            'file.cpp',             #dummy
            #'filestream.cpp',      #segfault
            'hash.cpp',
            #'http.cpp',            #free(), invallid pointer
            'httpserver.cpp',
            'httpstream.cpp',       #dummy
            #'messages.cpp',        #segfault
            'nfs.cpp',              #dummy
            'nfsstream.cpp',        #dummy
            'path.cpp',
            'sharedmessages.cpp',   #dummy
            'streaminterface.cpp',  #dummy
            #'taskmanager.cpp',     #segfault
            'utils.cpp',
            'websocket.cpp',        #dummy
            'websocketclient.cpp',  #dummy

            'js.cpp',               #dummy
            'jsdb.cpp',             #dummy
            'jsconsole.cpp',        #dummy
            'jsdebug.cpp',          #dummy
            'jsdebugger.cpp',       #dummy
            #'jsexposer.cpp',       #segfault
            #'jsfileio.cpp',        #segfault
            #'jsfs.cpp',            #segfault
            'jshttp.cpp',           #dummy
            'jshttpserver.cpp',     #dummy
            'jsnfs.cpp',            #dummy
            #'jsmodules.cpp',       #fails, probably due to incorrect testdefinition, sandbox
            'jsprocess.cpp',        #dummy
            'jssocket.cpp',         #dummy
            'jsstream.cpp',         #dummy
            #'jsthread.cpp',        #segfault
            'jsutils.cpp',
            #'jswebsocket.cpp',     #segfault
            #'jswebsocketclient.cpp', #fails, probaly due to no listening port
        ],
    }]
}
