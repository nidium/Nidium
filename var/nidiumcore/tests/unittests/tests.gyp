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
            'path.cpp',
            'args.cpp',
            'db.cpp',
            'events.cpp',
            'file.cpp',
            #'filestream.cpp',
            'hash.cpp',
            #'http.cpp',
            'httpserver.cpp',
            #'httpstream.cpp',
            #'messages.cpp',
            'nfs.cpp',
            'nfsstream.cpp',
            'path.cpp',
            'sharedmessages.cpp',
            #'streaminterface.cpp',
            #'taskmanager.cpp',
            'utils.cpp',
            'websocket.cpp',

            'js.cpp',
            'jsconsole.cpp',
            'jsdebug.cpp',
            #'jsexposer.cpp',
            #'jsfileio.cpp',
            #'jsfs.cpp',
            'jshttp.cpp',
            #'jshttpserver.cpp',
            #'jsmodules.cpp',
            #'jsprocess.cpp',
            #'jsprofiler.cpp',
            'jssocket.cpp',
            'jsstream.cpp',
            #'jsthread.cpp',
            'jsutils.cpp',
            #'jswebsocket.cpp',
            'path.cpp',
        ],
    }]
}
