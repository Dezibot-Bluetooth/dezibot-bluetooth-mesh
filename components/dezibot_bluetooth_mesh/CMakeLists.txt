FILE(GLOB_RECURSE app_sources "${CMAKE_SOURCE_DIR}/components/dezibot_bluetooth_mesh/*.cpp")
idf_component_register(
        SRCS ${app_sources}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/components/dezibot_bluetooth_mesh/"
        REQUIRES arduino bt nvs_flash
)
