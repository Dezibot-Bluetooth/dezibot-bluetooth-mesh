FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
set(apis bt nvs_flash lib)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${apis})
