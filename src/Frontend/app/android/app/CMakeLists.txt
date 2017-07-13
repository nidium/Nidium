# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

set(nidiumDir "${PROJECT_SOURCE_DIR}/../../../../../")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-mismatched-tags -Wno-invalid-offsetof -Wno-c++0x-extensions -fexceptions  -I${nidiumDir}/src/ -I${nidiumDir}/src/libapenetwork/src -I${nidiumDir}/third-party/c-ares -I${nidiumDir}/third-party/openssl/include -I${nidiumDir}/third-party/zlib -I${nidiumDir}/third-party/mozilla-central/js/src/obj/dist/include -I${nidiumDir}/third-party/mozilla-central/js/src -I${nidiumDir}/third-party/mozilla-central/nsprpub/dist/include/nspr -I${nidiumDir}/third-party/http-parser -I${nidiumDir}/third-party/leveldb/include -I${nidiumDir}/third-party/jsoncpp/dist -I${nidiumDir}/third-party/rapidxml -include ${nidiumDir}/build/third-party/js-config.h")

link_directories(
    ${nidiumDir}/build/third-party/
    ${nidiumDir}/build/out/Release/
    ${nidiumDir}/third-party/skia/out/config/android-arm_v7_thumb/Release/
    ${nidiumDir}third-party/skia/out/config/android-arm_v7_thumb/Release/obj/gyp/
)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/native-lib.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.


target_link_libraries( # Specifies the target library.
                       native-lib
                       nidium_android
                       nidium
                       nidiumcore
                       network

                       -Wl,--start-group
                       skia_images
                       skia_sfnt
                       skia_skgpu
                       skia_utils
                       skia_ports
                       skia_core
                       skia_effects
                       skia_opts
                       skia_opts_neon
                       etc1
                       gif
                       webp_dec
                       webp_enc
                       webp_utils
                       webp_dsp
                       webp_dsp_neon
                       cpu_features
                       expat
                       -Wl,--end-group

                       SDL2
                       zip
                       translator
                       translator_lib
                       preprocessor
                       angle_common
                       cares
                       http_parser
                       js_static
                       mozglue
                       leveldb
                       z
                       ssl
                       crypto
                       nspr4
                       freetype_static
                       jpeg
                       png
                      m
                      EGL
                      GLESv2
                       ${log-lib}
                       android
                       )