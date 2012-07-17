SKIA=/Users/anthonycatel/src/skia/trunk
CXX=g++
CPPFLAGS=-c -g -Wall -I../mozilla/js/src/dist/include/ \
				-I$(SKIA)/include/core 		\
				-I$(SKIA)/include/pipe 		\
				-I$(SKIA)/include/gpu 		\
				-I$(SKIA)/include/utils 	\
				-I$(SKIA)/include/device 	\
				-I$(SKIA)/include/config 	\
				-I$(SKIA)/include/images 	\
				-I$(SKIA)/include/test 		\
				-I$(SKIA)/include/pdf		\
				-I$(SKIA)/src/gpu/gl		\
				-I$(SKIA)/src/gpu			\
				-I$(SKIA)/src/core			\
				-I$(SKIA)/include/effects	\
				-I$(SKIA)/include/utils/mac

AR=ar
ARFLAGS=rcs
SOURCES=NativeJS.cpp NativeSkia.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativestudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS)

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
