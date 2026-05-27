cmake_minimum_required(VERSION 3.20)
project(jeffe-lang)

include(cmake/c_with_classes_mode.cmake)

add_library(jeffe-lang
        inc/jeffe_lang.h
        src/value.cpp
        src/core_ops.cpp
        src/value.hpp
        src/cstruct.hpp
        src/objmeta.hpp
        src/err.cpp
        src/meta_ops.cpp)
target_include_directories(jeffe-lang PUBLIC inc/)
get_target_property(JEFFE_LIB_TYPE jeffe-lang TYPE)
if(JEFFE_LIB_TYPE STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(jeffe-lang PUBLIC JEFFE_STATIC)
else()
    target_compile_definitions(jeffe-lang PRIVATE JEFFE_EXPORT)
endif()

c_with_classes_mode(jeffe-lang)

if(${PROJECT_IS_TOP_LEVEL})
    add_subdirectory(test)
endif()