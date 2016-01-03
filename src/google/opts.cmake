find_package(Jsoncpp REQUIRED)

include_directories( ${Jsoncpp_INCLUDES})
target_link_libraries(${NAME} ${Jsoncpp_LIBRARIES})
