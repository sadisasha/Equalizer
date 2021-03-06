# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2013-2014 Stefan.Eilemann@epfl.ch

include(CMakeParseArguments)
include(PurpleInstallPDB)

macro(EQ_ADD_TOOL NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;LINK_LIBRARIES" ${ARGN})
  set(THIS_TARGET ${NAME})
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  target_link_libraries(${THIS_TARGET} ${THIS_LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Tools")
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  purple_install_pdb(${THIS_TARGET} DESTINATION bin COMPONENT apps)
endmacro(EQ_ADD_TOOL NAME)

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/examples/include
  ${PROJECT_SOURCE_DIR}/examples)
add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers
list(APPEND CPPCHECK_EXTRA_ARGS -I${PROJECT_SOURCE_DIR}/examples/include
  -I${PROJECT_SOURCE_DIR}/examples)

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
  set(GLEW_LIBRARY ${GLEW_MX_LIBRARIES})
else()
  set(GLEW_LIBRARY "GLEW_MX_Equalizer")
  add_definitions(-DGLEW_STATIC=1 -DGLEW_MX=1)
endif()

if(WIN32)
  eq_add_tool(affinityCheck SOURCES affinityCheck/affinityCheck.cpp
    LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY}
    )
endif(WIN32)

eq_add_tool(eqThreadAffinity SOURCES threadAffinity/threadAffinity.cpp
  LINK_LIBRARIES Equalizer EqualizerServer
  )

eq_add_tool(eqPlyConverter
  HEADERS
  SOURCES eqPlyConverter/main.cpp
  LINK_LIBRARIES Equalizer triply
  )

eq_add_tool(eqWindowAdmin
  SOURCES windowAdmin/main.cpp
  LINK_LIBRARIES EqualizerAdmin
  )

eq_add_tool(eqServer
  SOURCES server/eqServer.cpp
  LINK_LIBRARIES EqualizerServer
  )

list(APPEND CPPCHECK_EXTRA_ARGS --suppress=invalidscanf
  --suppress=invalidscanf_libc
  --suppress=variableScope --suppress=invalidPointerCast
  --suppress=invalidPrintfArgType_sint) # Yes, it's that bad.

eq_add_tool(eVolveConverter
  HEADERS
    eVolveConverter/codebase.h
    eVolveConverter/ddsbase.h
    eVolveConverter/eVolveConverter.h
    eVolveConverter/hlp.h
  SOURCES
    eVolveConverter/eVolveConverter.cpp
    eVolveConverter/ddsbase.cpp
  LINK_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
