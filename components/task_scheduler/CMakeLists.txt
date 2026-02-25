file(GLOB_RECURSE LIB_SRCS "${CMAKE_SOURCE_DIR}/arduino_components/task_scheduler/src/*.cpp")
idf_component_register(
        SRCS ${LIB_SRCS}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/arduino_components/task_scheduler/src/"
        REQUIRES arduino
)
