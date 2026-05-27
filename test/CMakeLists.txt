include(CTest)
enable_testing()

add_executable(jeffe-lang-test main.cpp doctest.h
        test_value.cpp test_meta.cpp test_core.cpp util.hpp)
target_link_libraries(jeffe-lang-test PUBLIC jeffe-lang)