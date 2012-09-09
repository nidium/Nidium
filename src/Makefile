SKIA=/Users/anthonycatel/src/skia/trunk
CXX=g++
CPPFLAGS= -O2 -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_BUILD_FOR_MAC -DSK_USE_POSIX_THREADS -DSK_RELEASE -DGR_RELEASE=1 -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden -c -g -Wall -I../mozilla/js/src/dist/include/ \
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
				-I$(SKIA)/include/utils/mac \
				-I../network

AR=ar
ARFLAGS=rcs
SOURCES=NativeJS.cpp NativeSkia.cpp NativeSkGradient.cpp NativeSkImage.cpp NativeShadowLooper.cpp NativeSharedMessages.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativestudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
