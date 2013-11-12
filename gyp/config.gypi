{
    'variables' : {
        'native_version': '0.1',
        'native_webgl%': 0,
        'native_audio%': 1,

        # Deprecated in favor of --strip option to deps.py 
        'native_strip_exec%': 1,

        'native_src_path': '../src/',
        'native_av_path': '../av/',
        'native_app_path': '../app/',
        'native_interface_path': '../interface/',
        'native_network_path': '../nativejscore/network/',
        'native_nativejscore_path': '../nativejscore/',
        'native_resources_path': '../resources/',
        'native_exec_name': 'nidium',
        'native_exec_path': '../framework/dist/',

        'third_party_path%': '../third-party/',
        'native_output%': '../out/',
        'addresse_sanitizer%': 0,

        'native_interface': 'auto', 

        # Crash reporter settings
        'native_enable_breakpad%': 0,
        'native_crash_collector_host': 'crash.nidium.com',
        'native_crash_collector_port': 80,
        'native_crash_collector_endpoint': '/submit',

        # Linux build only
        'native_use_gtk': 1,
        'native_use_qt': 0,
    },
}
