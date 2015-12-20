set (CMAKE_CXX_STANDARD 11)

find_path(BOT bot.h HINTS ../../BakaBot/src HINTS ../../src NO_DEFAULT_PATH)

include_directories(
	${BOT}
	${BOT}/event
    $ENV{COREPATH}/include/plugin
    $ENV{COREPATH}/include/sock
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
endforeach(NAME)
