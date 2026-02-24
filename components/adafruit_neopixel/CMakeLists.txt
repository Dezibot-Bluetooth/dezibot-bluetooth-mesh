file(GLOB_RECURSE LIB_SRCS "${CMAKE_SOURCE_DIR}/arduino_libraries/adafruit_neopixel/Adafruit_NeoPixel.cpp")
idf_component_register(
        SRCS ${LIB_SRCS}
        INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/arduino_libraries/adafruit_neopixel"
        REQUIRES arduino
)
