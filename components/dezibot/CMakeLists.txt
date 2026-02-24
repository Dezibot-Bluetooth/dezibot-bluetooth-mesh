file(GLOB_RECURSE LIB_SRCS "${CMAKE_SOURCE_DIR}/arduino_libraries/dezibot/src/*.cpp")
idf_component_register(
        SRCS ${LIB_SRCS}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/arduino_libraries/dezibot/src/"
        REQUIRES arduino adafruit_neopixel painlessMesh async_tcp veml6040
)
