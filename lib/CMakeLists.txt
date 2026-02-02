set(app_sources 
    ${CMAKE_SOURCE_DIR}/lib/client.c
    ${CMAKE_SOURCE_DIR}/lib/bluetooth.c
)
set(apis bt nvs_flash)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${apis})
