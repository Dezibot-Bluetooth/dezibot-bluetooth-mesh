FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/lib/*.*)
set(apis bt nvs_flash)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${apis})
