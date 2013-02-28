#!/bin/sh
echo "Downloading ffmpeg...\n"
curl http://www.ffmpeg.org/releases/ffmpeg-1.1.2.tar.bz2 | tar xj && mv ffmpeg-1.1.2 ffmpeg
echo "Building ffmpeg....\n"
cd ffmpeg && ./configure --disable-avdevice --disable-postproc --disable-avfilter --disable-programs --disable-ffplay --disable-ffserver --disable-ffprobe --disable-everything --enable-decoder=mp3,vorbis,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8 --enable-parser=vorbis,mpegaudio --enable-demuxer=mp3,ogg,vorbis,pcm_alaw,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_mulaw,pcm_s16be,pcm_s16le,pcm_s24be,pcm_s24le,pcm_s32be,pcm_s32le,pcm_s8,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8 && make
cd ../
echo "Downloading portaudio...\n"
curl http://www.portaudio.com/archives/pa_stable_v19_20111121.tgz |tar zx
echo "Building portaudio...\n"
cd portaudio && ./configure && make
cd ../
echo "Downloading zita-resampler...\n"
curl http://kokkinizita.linuxaudio.org/linuxaudio/downloads/zita-resampler-1.3.0.tar.bz2 |tar xj && mv zita-resampler-1.3.0 zita-resampler
echo "Patching zita-resampler Makefile...\n"
patch -p0 < zita.patch
echo "Building zita-resampler...\n"
cd zita-resampler/libs/ && make
cd ../
echo "All done!"
