{
    'targets': [{
        'target_name': 'nidium-crash-reporter',
        'type': 'executable',
        'product_dir': '../framework/dist/',
        'defines': [
            'NATIVE_VERSION=<(native_version)',
            'NATIVE_BUILD=<!@git rev-parse HEAD)'
        ],
        'sources': [
            '<(native_src_path)/NativeCrashReporter.c',
        ],
    }],
}
