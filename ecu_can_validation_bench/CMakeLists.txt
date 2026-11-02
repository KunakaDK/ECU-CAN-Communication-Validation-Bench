cmake_minimum_required(VERSION 3.16)
project(ecu_can_validation_bench VERSION 1.0.0 LANGUAGES CXX)

option(ECU_CAN_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(ECU_CAN_BUILD_TESTS "Build the automated test suite" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ecu_can_core
    src/frame_validator.cpp
    src/virtual_can_bus.cpp
    src/ecu.cpp
)
target_include_directories(ecu_can_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ECU_CAN_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(ecu_can_core PRIVATE /W4 /permissive-)
    else()
        target_compile_options(ecu_can_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
endif()

add_executable(ecu_can_bench src/main.cpp)
target_link_libraries(ecu_can_bench PRIVATE ecu_can_core)

if(ECU_CAN_BUILD_TESTS)
    enable_testing()
    add_executable(ecu_can_tests
        tests/test_main.cpp
        tests/test_frame_validator.cpp
        tests/test_virtual_can_bus.cpp
        tests/test_ecu.cpp
    )
    target_include_directories(ecu_can_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(ecu_can_tests PRIVATE ecu_can_core)
    if(ECU_CAN_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(ecu_can_tests PRIVATE /W4 /permissive-)
        else()
            target_compile_options(ecu_can_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        endif()
    endif()
    add_test(NAME ecu_can_tests COMMAND ecu_can_tests)
endif()
