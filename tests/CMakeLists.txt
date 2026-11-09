# The C API must compile as plain C99 with strict warnings.
add_executable(sdk_c_test sdk_c_test.c)
set_target_properties(sdk_c_test PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_link_libraries(sdk_c_test PRIVATE torustiq::sdk)

# The C++ helpers must compile as C++20.
add_executable(sdk_cpp_test sdk_cpp_test.cpp)
set_target_properties(sdk_cpp_test PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
                                              CXX_EXTENSIONS OFF)
target_link_libraries(sdk_cpp_test PRIVATE torustiq::sdk)

foreach(t sdk_c_test sdk_cpp_test)
    if(MSVC)
        target_compile_options(${t} PRIVATE /W4)
    else()
        target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME ${t} COMMAND ${t})
endforeach()
