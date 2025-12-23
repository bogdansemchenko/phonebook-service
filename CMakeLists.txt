cmake_minimum_required(VERSION 3.20)

project(phonebook-service
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(oatpp CONFIG REQUIRED)
find_package(oatpp-swagger CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    oatpp::oatpp
    oatpp::oatpp-swagger
    Threads::Threads
)


set(UI_DIR "${CMAKE_SOURCE_DIR}/swagger-res")

file(COPY ${UI_DIR} DESTINATION ${CMAKE_BINARY_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SWAGGER_RES_PATH="swagger-res"
)


enable_testing()
if(EXISTS "${CMAKE_SOURCE_DIR}/test/app_test.cpp")
    file(GLOB_RECURSE TEST_LIB_SOURCES "src/*.cpp")
    list(FILTER TEST_LIB_SOURCES EXCLUDE REGEX "src/app.cpp")

    add_executable(run_tests test/app_test.cpp ${TEST_LIB_SOURCES})
    target_include_directories(run_tests PRIVATE src)
    target_compile_definitions(run_tests PRIVATE SWAGGER_RES_PATH="${CMAKE_BINARY_DIR}/swagger-res")
    target_link_libraries(run_tests PRIVATE 
        GTest::gtest_main
        oatpp::oatpp
        oatpp::oatpp-swagger
        Threads::Threads
    )
    add_test(NAME PhonebookTests COMMAND run_tests)
endif()



if(EXISTS "${CMAKE_SOURCE_DIR}/test/benchmark_test.cpp")
    file(GLOB_RECURSE BENCH_LIB_SOURCES "src/*.cpp")
    list(FILTER BENCH_LIB_SOURCES EXCLUDE REGEX "src/app.cpp")

    add_executable(run_benchmarks test/benchmark_test.cpp ${BENCH_LIB_SOURCES})
    target_include_directories(run_benchmarks PRIVATE src)
    target_compile_definitions(run_benchmarks PRIVATE SWAGGER_RES_PATH="${CMAKE_BINARY_DIR}/swagger-res")
    
    target_link_libraries(run_benchmarks PRIVATE 
        GTest::gtest_main
        oatpp::oatpp
        oatpp::oatpp-swagger
        Threads::Threads
    )


endif()