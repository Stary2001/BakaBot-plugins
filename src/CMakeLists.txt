set (CMAKE_CXX_STANDARD 11)

find_path(BOT bot.h HINTS ../../BakaBot/src HINTS ../../src NO_DEFAULT_PATH)

include_directories(
	${BOT}
	${BOT}/event
    $ENV{COREPATH}/include/plugin
    $ENV{COREPATH}/include/sock
)

file(GLOB_RECURSE SOURCES "*.cpp")
foreach(F ${SOURCES})
  string(REGEX MATCH "([^/]+)$" F2 ${F})
  string(REPLACE ".cpp" "" NAME ${F2})
  add_library(${NAME} MODULE ${F})
  SET_TARGET_PROPERTIES(${NAME} PROPERTIES PREFIX "")
endforeach(F)
