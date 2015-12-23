find_package(Jsoncpp REQUIRED)
find_package(CURL REQUIRED)

include_directories(${Jsoncpp_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
target_link_libraries(${NAME} ${Jsoncpp_LIBRARIES} ${CURL_LIBRARIES})
