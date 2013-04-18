{
    'variables' : {
        'native_webgl%': 0,
        'native_audio%': 1,

        'native_strip_exec%': 1,

        'native_src_path%': '../src/',
        'native_av_path%': '../av/',
        'native_app_path%': '../app/',
        'native_interface_path%': '../interface/',
        'native_network_path%': '../network/',
        'native_exec_name%': 'nativeapp',

        'third_party_path%': '../third-party/',
        'native_output%': '../out/',

        # Linux build only
        'native_use_gtk': 1,
        'native_use_qt': 0,

        # thoses settings are not working
        'native_third_party_build': 'debug',
        'native_autoconf_path%': '/usr/bin/autoconf2.13',

    },
}
