cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)
pkg_check_modules(FYAML libfyaml)

include_directories(${FYAML_INCLUDE_DIRS})

add_executable(fy fyaml.c)
target_link_libraries(fy ${FYAML_LIBRARIES})