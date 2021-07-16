cmake_minimum_required (VERSION 2.8.11)
project (egl-wrapper)

find_package(X11)

set(SOURCES
    egl-wrapper.c
    egl-wrapper.h
)

add_library(egl-wrapper SHARED egl-wrapper.c)
target_link_libraries(egl-wrapper PUBLIC dl)
target_include_directories(egl-wrapper PUBLIC ${CMAKE_SOURCE_DIR})

add_subdirectory(examples)