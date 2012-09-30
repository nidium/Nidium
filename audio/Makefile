CXX=~/.vim/bin/cc_args.py g++
CPPFLAGS=-I./ -c -g -Wall -D__STDC_CONSTANT_MACROS

AR=ar
ARFLAGS=rcs
C_SOURCES=pa_ringbuffer.c
SOURCES=NativeAudio.cpp 
OBJECTS=$(C_SOURCES:.c=.o) $(SOURCES:.cpp=.o)
EXECUTABLE=libnativeaudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
