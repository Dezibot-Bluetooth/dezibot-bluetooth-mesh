file(GLOB_RECURSE LIB_SRCS "${CMAKE_SOURCE_DIR}/arduino_components/dezibot/src/*.cpp")
idf_component_register(
        SRCS ${LIB_SRCS}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/arduino_components/dezibot/src/"
        REQUIRES arduino adafruit_neopixel painlessMesh async_tcp veml6040
)

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-error=format
        -Wno-error=parentheses
        -Wno-error=int-in-bool-context
        -Wno-error=char-subscripts
)
