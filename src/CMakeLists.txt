set (CMAKE_CXX_STANDARD 11)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules)

find_path(BOT bot.h HINTS ../../BakaBot/src HINTS ../../src HINTS ../../bakabot/src NO_DEFAULT_PATH)
find_path(SOCK include/connection.h HINTS ../../corelibs/libsock ../../../corelibs/libsock NO_DEFAULT_PATH)
find_path(PLUGIN include/plugin.h HINTS ../../corelibs/libplugin ../../../corelibs/libplugin NO_DEFAULT_PATH)

if(${SOCK} STREQUAL "SOCK-NOTFOUND")
  message(FATAL_ERROR "libsock not found")
endif()

if(${PLUGIN} STREQUAL "PLUGIN-NOTFOUND")
  message(FATAL_ERROR "libplugin not found")
endif()

if(${BOT} STREQUAL "BOT-NOTFOUND")
  message(FATAL_ERROR "Could not find BakaBot folder!")
endif()

include_directories(
	${BOT}
	${BOT}/event
    ${BOT}/commands
    ${PLUGIN}/include
    ${SOCK}/include
)

# http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(DIRS ${CMAKE_CURRENT_SOURCE_DIR})

foreach(NAME ${DIRS})
  file(GLOB_RECURSE ${NAME}SOURCES ${NAME}/*)

  add_library(${NAME} MODULE ${${NAME}SOURCES})
  SET_TARGET_PROPERTIES(${NAME} PROPERTIES PREFIX "")

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/opts.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/opts.cmake)
  endif()
endforeach(NAME)

install(TARGETS ${DIRS} LIBRARY DESTINATION ${BOT}/../run)
