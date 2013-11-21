{
    'targets': [
        {
            'target_name': 'jsoncpp',
            'type': 'static_library',
            'sources': [
                '<(third_party_path)/jsoncpp/include/json/assertions.h',
                '<(third_party_path)/jsoncpp/include/json/autolink.h',
                '<(third_party_path)/jsoncpp/include/json/config.h',
                '<(third_party_path)/jsoncpp/include/json/features.h',
                '<(third_party_path)/jsoncpp/include/json/forwards.h',
                '<(third_party_path)/jsoncpp/include/json/json.h',
                '<(third_party_path)/jsoncpp/include/json/reader.h',
                '<(third_party_path)/jsoncpp/include/json/value.h',
                '<(third_party_path)/jsoncpp/include/json/writer.h',
                '<(third_party_path)/jsoncpp/src/lib_json/json_batchallocator.h',
                '<(third_party_path)/jsoncpp/src/lib_json/json_reader.cpp',
                '<(third_party_path)/jsoncpp/src/lib_json/json_tool.h',
                '<(third_party_path)/jsoncpp/src/lib_json/json_value.cpp',
                '<(third_party_path)/jsoncpp/src/lib_json/json_writer.cpp',
            ],
            'include_dirs': [
                '<(third_party_path)/jsoncpp/include/',
                '<(third_party_path)/jsoncpp/src/lib_json/',
            ],
        },
    ],
}
