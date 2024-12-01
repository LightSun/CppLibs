
#include_directories(./)

if(NOT MED_ACC_DIR)
    add_compile_definitions(DISABLE_ONNX2TRT)
endif()

## OnnxPaser

## lib
file(GLOB_RECURSE SRC_MODEL_M
    src/*.h
    src/*.hpp
    src/*.cpp
    src/*.c
    )

add_library(ModelM
    ${SRC_MODEL_M}
    )
target_link_libraries(ModelM PUBLIC
    ${MED_ACC_LIB}
    snappy
    )
if(ONNX_PARSER_DIR)
    target_include_directories(ModelM PUBLIC ${ONNX_PARSER_DIR}/include)
    target_compile_definitions(ModelM PUBLIC WITH_ONNX_PARSER)
    target_link_libraries(ModelM PUBLIC ${ONNX_PARSER_DIR}/lib/libSimpleOnnxParser.so)
endif(ONNX_PARSER_DIR)

if(WIN32)
    target_link_libraries(ModelM PUBLIC shlwapi)
endif(WIN32)

# shared_lib
add_library(ModelM_shared SHARED
    ${SRC_MODEL_M}
    )
target_link_libraries(ModelM_shared PUBLIC
    snappy
    )
target_compile_definitions(ModelM_shared PUBLIC DISABLE_ONNX2TRT)
if(WIN32)
    target_link_libraries(ModelM_shared PUBLIC shlwapi)
endif(WIN32)

## main
add_executable(ModelMMain
     main/main.cpp
     )
target_link_libraries(ModelMMain PUBLIC
    ModelM
    ${OpenCV_LIBS}
    #${ABSL_TARGETS}
    )
## wrapper
file(GLOB_RECURSE SRC_LLAMAMAIN_SRC
    llama_main/*.h
    llama_main/*.hpp
    llama_main/*.cpp
    llama_main/*.c
    )
add_executable(MedEntry
    ${SRC_LLAMAMAIN_SRC}
    )
target_link_libraries(MedEntry PUBLIC
    ModelM_shared
    ${OpenCV_LIBS}
    )
