#!/bin/sh

# Make plugin look in its private Frameworks directory for these frameworks.

/usr/bin/install_name_tool -change \
  @rpath/SDL2.framework/Versions/A/SDL2 \
  @loader_path/../Frameworks/SDL2.framework/Versions/A/SDL2\
  "${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}"
