cmake_minimum_required(VERSION 3.16.0)

set(EXTRA_COMPONENT_DIRS
  "${CMAKE_SOURCE_DIR}/src"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dezibot-bluetooth-mesh)

find_package(Doxygen)

if (DOXYGEN_FOUND)
    add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
endif()