function(c_with_classes_mode TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "c_with_classes_mode: '${TARGET_NAME}' is not a valid CMake target.")
    endif()

    # Check for MSVC CLI syntax compatibility (Native MSVC and Clang-CL)
    if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        target_compile_options(${TARGET_NAME} PRIVATE 
            /EHs-c-  # Disable C++ exceptions completely
            /GR-     # Disable RTTI
        )
        # Prevent standard library headers from pulling in exception handling structures
        target_compile_definitions(${TARGET_NAME} PRIVATE _HAS_EXCEPTIONS=0)
    else()
        # Standard Clang / GCC for Linux and macOS
        target_compile_options(${TARGET_NAME} PRIVATE 
            -fno-exceptions 
            -fno-rtti
        )
    endif()

    message(STATUS "Configured target '${TARGET_NAME}' for C-with-classes mode.")
endfunction()