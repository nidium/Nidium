# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

{
    'include_dirs': [
        '<(nidium_src_path)',
        '<(third_party_path)/portaudio/src/common/',
        '<(third_party_path)/portaudio/include/',
        '<(third_party_path)/zita-resampler/libs/',
        '<(third_party_path)/ffmpeg/',
        '<(third_party_path)/libcoroutine/source/',
        '<(third_party_path)/basekit/source/',
    ],
    'dependencies': [
        '<(nidium_network_path)/gyp/network.gyp:network-includes',
        'libnidiumcore.gyp:libnidiumcore-includes',
    ],
    'sources': [
        '<(nidium_av_path)AV.cpp',
        '<(nidium_av_path)AudioNode.cpp',
        '<(nidium_av_path)Audio.cpp',
        '<(nidium_av_path)Video.cpp',
        '<(nidium_av_path)AudioNodeGain.cpp',
        '<(nidium_av_path)AudioNodeDelay.cpp',
    ],
}

