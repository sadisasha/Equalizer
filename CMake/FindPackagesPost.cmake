
# Copyright (c) 2013 Stefan Eilemann <eile@eyescale.ch>

# additional compile-time definitions
if(HWLOC_GL_FOUND)
  list(APPEND FIND_PACKAGES_DEFINES EQUALIZER_USE_HWLOC_GL)
endif()

if(MAGELLAN_FOUND AND NOT EQ_AGL_USED)
  list(APPEND FIND_PACKAGES_DEFINES EQUALIZER_USE_MAGELLAN_GLX)
endif()

list(APPEND FIND_PACKAGES_DEFINES GLEW_MX) # always define GLEW_MX
list(APPEND FIND_PACKAGES_DEFINES GLEW_NO_GLU)
if(GLEW_MX_FOUND)
  list(APPEND FIND_PACKAGES_DEFINES EQ_FOUND_GLEW_MX)
else()
  list(APPEND FIND_PACKAGES_DEFINES GLEW_STATIC EQ_GLEW_INTERNAL)
endif()

if(WIN32)
  list(APPEND FIND_PACKAGES_DEFINES WGL)
endif()
if(EQ_GLX_USED)
  list(APPEND FIND_PACKAGES_DEFINES GLX)
endif()