file(GLOB_RECURSE LIB_SRCS "${CMAKE_SOURCE_DIR}/arduino_libraries/painlessMesh/src/*.cpp")
idf_component_register(
        SRCS ${LIB_SRCS}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/arduino_libraries/painlessMesh/src/"
        REQUIRES arduino async_tcp arduino_json task_scheduler
)
