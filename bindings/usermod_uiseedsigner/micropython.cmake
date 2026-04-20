add_library(usermod_uiseedsigner INTERFACE)

target_sources(usermod_uiseedsigner INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mod_uiseedsigner.c
)

target_include_directories(usermod_uiseedsigner INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_LIST_DIR}/../../config
)

target_link_libraries(usermod INTERFACE usermod_uiseedsigner)
