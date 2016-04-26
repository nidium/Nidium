{
    'targets': [{
        'target_name': 'nidium-crash-reporter',
        'type': 'executable',
        'product_dir': '../framework/dist/',
        'sources': [
            '<(nidium_app_path)/linux/CrashReporter.c',
        ],
    }],
}
