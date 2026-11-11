configure_file(keek_config.h.in keek_config.h @ONLY)

add_library(keekconfig INTERFACE)
target_include_directories(keekconfig INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(core)
add_subdirectory(daemon)
