SKIA=/home/efyx/dev/NativeStudio/skia-read-only/
CXX=g++

#Linux CPPFLAGS
#CPPFLAGS= -O2 -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_SAMPLES_FOR_X -DSK_BUILD_FOR_UNIX -DSK_USE_POSIX_THREADS -DSK_SUPPORT_PDF -DGR_LINUX_BUILD=1 -DSK_RELEASE -DGR_RELEASE=1 -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden -c -g -Wall -I../mozilla/js/src/dist/include/ \

#MACOS CPPFLAGS
CPPFLAGS= -O2 -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_BUILD_FOR_MAC -DSK_USE_POSIX_THREADS -DGR_MAC_BUILD=1 -DSK_SUPPORT_PDF -DSK_RELEASE -DGR_RELEASE=1 -DNDEBUG -DN_DEBUG -fvisibility=hidden -fvisibility-inlines-hidden -c -g -Wall -I../mozilla/js/src/dist/include/  \
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
SOURCES=NativeJS.cpp NativeSkia.cpp NativeSkGradient.cpp NativeSkImage.cpp ape_array.cpp ape_pool.cpp ape_buffer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativestudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
