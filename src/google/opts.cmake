find_package(Jsoncpp REQUIRED)

include_directories( ${Jsoncpp_INCLUDE_DIRS})
target_link_libraries(${NAME} ${Jsoncpp_LIBRARIES})
