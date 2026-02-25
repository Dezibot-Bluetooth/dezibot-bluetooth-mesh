FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
set(apis arduino dezibot dezibot_bluetooth_mesh)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${apis})
