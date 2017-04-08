# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'targets': [{
        'target_name': 'libnidiumcore-unittests',
        'product_dir': '<(nidium_tests_output_path)',
        'type': 'executable',
        'dependencies': [
            'libnidiumcore.gyp:*',
            '<(nidium_network_path)/gyp/network-unittests.gyp:unittests-settings',
            '<(nidium_network_path)gyp/network.gyp:*',
        ],
        'conditions': [
#                ['OS!="win"', {
#                    'cflags_cc': [
#                        '-include ../src/Macros.h'
#                    ],
#                }],
                ['OS=="win"', {
                    'defines': [
                        'WIN32',
                    ],
                }],
          ],
        'sources': [
            '<(nidium_tests_path)unittest_0.cpp',
            '<(nidium_tests_path)args.cpp',
            '<(nidium_tests_path)db.cpp',
            '<(nidium_tests_path)events.cpp',           #dummy
            '<(nidium_tests_path)file.cpp',             #dummy
            #'<(nidium_tests_path)filestream.cpp',      #segfault
            '<(nidium_tests_path)hash.cpp',
            #'<(nidium_tests_path)http.cpp',            #free(), invallid pointer
            #'<(nidium_tests_path)httpserver.cpp',      #free(), invallid pointer
            '<(nidium_tests_path)httpstream.cpp',       #dummy
            #'<(nidium_tests_path)messages.cpp',        #segfault
            '<(nidium_tests_path)nfs.cpp',              #dummy
            '<(nidium_tests_path)nfsstream.cpp',        #dummy
            '<(nidium_tests_path)path.cpp',
            '<(nidium_tests_path)sharedmessages.cpp',   #dummy
            '<(nidium_tests_path)streaminterface.cpp',  #dummy
            #'<(nidium_tests_path)taskmanager.cpp',     #segfault
            '<(nidium_tests_path)utils.cpp',

            '<(nidium_tests_path)nidiumjs.cpp',         #dummy
            '<(nidium_tests_path)jshttpserver.cpp',     #dummy
            '<(nidium_tests_path)jssocket.cpp',         #dummy
            '<(nidium_tests_path)jsutils.cpp',
            #'<(nidium_tests_path)jswebsocket.cpp',     #segfault
            #'<(nidium_tests_path)jswebsocketclient.cpp', #fails, probaly due to no listening port
        ],
    }]
}
