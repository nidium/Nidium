UNAME := $(shell uname)
ENABLE_WEBGL=0

ifeq ($(UNAME), Linux)
SKIA=../third-party/skia/
NATIVEFLAGS=-DEXPORT_JS_API -DIMPL_MFBT -DUSE_SYSTEM_MALLOC=1 -DENABLE_ASSEMBLER=1 -DENABLE_JIT=1 -DEXPORT_JS_API -DIMPL_MFBT  -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_SAMPLES_FOR_X -DSK_BUILD_FOR_UNIX -DSK_USE_POSIX_THREADS -DSK_SUPPORT_PDF -DGR_LINUX_BUILD=1 -DSK_RELEASE -DGR_RELEASE=1 -DDEBUG -D_DEBUG -Wno-invalid-offsetof
endif

ifeq ($(UNAME), Darwin)
SKIA=/Users/anthonycatel/src/skia/trunk
NATIVEFLAGS=-DENABLE_TYPEDARRAY_MOVE -DENABLE_YARR_JIT=1 -DNO_NSPR_10_SUPPORT -DIMPL_MFBT -DEXPORT_JS_API -Qunused-arguments -DMOZILLA_CLIENT -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_REDEFINE_ROOT2OVER2_TO_MAKE_ARCTOS_CONVEX -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SUPPORT_GPU=1 -DSK_BUILD_FOR_MAC -DSK_USE_POSIX_THREADS -DGR_MAC_BUILD=1 -DSK_SUPPORT_PDF -DSK_RELEASE -DGR_RELEASE=1 -DTRACING -DJS_THREADSAFE -fvisibility=hidden -fvisibility-inlines-hidden -Wno-invalid-offsetof
endif

CXX=g++
CPPFLAGS=-g $(NATIVEFLAGS) -c -g -Wall -I../mozilla/js/src/dist/include/ \
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
		NativeFileIO.cpp 			\
		NativeJSWindow.cpp 			\
		NativeJSCanvas.cpp 			\
		NativeCanvasHandler.cpp 	\
		NativeCanvas2DContext.cpp   \
		NativeJSFileIO.cpp

ifeq ($(ENABLE_WEBGL), 1)
	CPPFLAGS += -I../third-party/angle/include
	SOURCES += NativeJSWebGL.cpp
	NATIVEFLAGS += -DWEBGL_ENABLED
endif

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=libnativestudio.a

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS)
		$(AR) $(ARFLAGS) $@ $(OBJECTS) 

.cpp.o:
		$(CXX) $(CPPFLAGS) $< -o $@
		
clean:
		rm -f $(OBJECTS) $(EXECUTABLE)
