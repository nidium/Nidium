{
    'targets': [{
        'target_name': 'nidium-crash-reporter',
        'type': 'executable',
        'product_dir': '../framework/dist/',
        'sources': [
            '<(native_app_path)/linux/NativeCrashReporter.c',
        ],
    }],
}
