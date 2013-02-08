#CXX=~/.vim/bin/cc_args.py g++
CXX=g++
LIBAV=../../ffmpeg/
PORTAUDIO=../../portaudio/
LIBRESAMPLER=../../zita-resampler-1.3.0/
CPPFLAGS=-I./ -I../network/ -I../src/ -I$(LIBAV) -I$(PORTAUDIO)/src/common/ -I$(LIBRESAMPLER) -c -g -Wall -D__STDC_CONSTANT_MACROS 

AR=ar
ARFLAGS=rcs
SOURCES=NativeAudioNode.cpp NativeAudio.cpp NativeVideo.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativeaudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
