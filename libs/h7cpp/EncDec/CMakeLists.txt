cmake_minimum_required(VERSION 3.0)

project(openssl_test C CXX)

find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MED_REL_FLAGS "-O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MED_REL_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -O2")
#fpic
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

## core

file(GLOB_RECURSE CORE_SRC "core/*.h" "core/*.cpp" "core/*.c")

add_library(med_rsa
    ${CORE_SRC}
    )

set(DEP_LIBS
    OpenSSL::Crypto OpenSSL::SSL  # modern
    #${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}  # old style
)
target_include_directories(med_rsa PUBLIC core/)
target_link_libraries(med_rsa ${DEP_LIBS} pthread)
## test
file(GLOB_RECURSE TEST_SRC "test/*.h" "test/*.cpp" "test/*.c")
add_executable(test_med_rsa
    ${TEST_SRC}
    )
target_link_libraries(test_med_rsa PUBLIC med_rsa)

