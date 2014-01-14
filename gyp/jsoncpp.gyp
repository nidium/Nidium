{
    'targets': [
        {
            'target_name': 'jsoncpp',
            'type': 'static_library',
            'sources': [
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/assertions.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/autolink.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/config.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/features.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/forwards.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/json.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/reader.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/value.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/json/writer.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/json_batchallocator.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/json_reader.cpp',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/json_tool.h',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/json_value.cpp',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/json_writer.cpp',
            ],
            'include_dirs': [
                '<(DEPTH)/<(third_party_path)/jsoncpp/include/',
                '<(DEPTH)/<(third_party_path)/jsoncpp/src/lib_json/',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '<(DEPTH)/<(third_party_path)/jsoncpp/include',
                ]
            }
        },
    ],
}
