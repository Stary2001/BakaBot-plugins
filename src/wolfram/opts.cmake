find_package(TinyXML REQUIRED)

include_directories( ${TINYXML_INCLUDE_DIRS})
target_link_libraries(${NAME} ${TINYXML_LIBRARIES})
