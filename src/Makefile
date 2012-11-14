UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
SKIA=/home/efyx/dev/NativeStudio/NativeStudio/skia-read-only
NATIVEFLAGS=-DEXPORT_JS_API -DIMPL_MFBT -DUSE_SYSTEM_MALLOC=1 -DENABLE_ASSEMBLER=1 -DENABLE_JIT=1 -DEXPORT_JS_API -DIMPL_MFBT  -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_SAMPLES_FOR_X -DSK_BUILD_FOR_UNIX -DSK_USE_POSIX_THREADS -DSK_SUPPORT_PDF -DGR_LINUX_BUILD=1 -DSK_RELEASE -DGR_RELEASE=1 -DNDEBUG -Wno-invalid-offsetof
endif

ifeq ($(UNAME), Darwin)
SKIA=/Users/anthonycatel/src/skia/trunk
NATIVEFLAGS=-DUSE_SYSTEM_MALLOC=1 -DENABLE_ASSEMBLER=1 -DENABLE_JIT=1 -DEXPORT_JS_API -DIMPL_MFBT -Qunused-arguments -DMOZILLA_CLIENT -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_BUILD_FOR_MAC -DSK_USE_POSIX_THREADS -DSK_RELEASE -DGR_RELEASE=1 -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden -Wno-invalid-offsetof
endif

CXX=g++
CPPFLAGS= -O2 $(NATIVEFLAGS) -c -g -O2 -Wall -I../mozilla/js/src/dist/include/ \
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
				-I../network \
				

AR=ar
ARFLAGS=rcs
SOURCES=NativeJS.cpp 				\
		NativeSkia.cpp 				\
		NativeSkGradient.cpp 		\
		NativeSkImage.cpp 			\
		NativeShadowLooper.cpp 		\
		NativeSharedMessages.cpp 	\
		NativeJSExposer.cpp 		\
		NativeJSSocket.cpp 			\
		NativeJSThread.cpp 			\
		NativeHTTP.cpp 				\
		NativeJSHttp.cpp 			\
		NativeJSImage.cpp 			\
		NativeJSNative.cpp 			\
		NativeFileIO.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativestudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
