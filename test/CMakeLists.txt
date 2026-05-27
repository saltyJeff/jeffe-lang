include(CTest)
enable_testing()

add_executable(jeffe-lang-test main.cpp doctest.h
        test_value.cpp)
target_link_libraries(jeffe-lang-test PUBLIC jeffe-lang)