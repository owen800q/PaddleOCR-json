cmake_minimum_required(VERSION 3.10)

project(PaddleOCR_json VERSION 1.2.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/installed)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

include(FetchContent)

FetchContent_Declare(
    autolog
    GIT_REPOSITORY https://gitee.com/Double_V/AutoLog
    GIT_TAG        main
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(autolog)
add_library(autolog INTERFACE)
target_include_directories(autolog INTERFACE ${autolog_SOURCE_DIR})
add_library(autolog::header ALIAS autolog)

FetchContent_Declare(
    paddle
    URL https://paddle-wheel.bj.bcebos.com/develop/win-infer/mkl/cpu/paddle_inference.zip
    URL_HASH MD5=b20e33128938d69bca106d49f6336c5c
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(paddle)
add_library(paddle_inference SHARED IMPORTED GLOBAL)

if(WIN32)
    set_target_properties(paddle_inference PROPERTIES
        IMPORTED_LOCATION ${paddle_SOURCE_DIR}/paddle/lib/paddle_inference.dll
        IMPORTED_IMPLIB ${paddle_SOURCE_DIR}/paddle/lib/paddle_inference.lib
    )
elseif(APPLE)
    set_target_properties(paddle_inference PROPERTIES
        IMPORTED_LOCATION ${paddle_SOURCE_DIR}/paddle/lib/libpaddle_inference.dylib
    )
else()
    set_target_properties(paddle_inference PROPERTIES
        IMPORTED_LOCATION ${paddle_SOURCE_DIR}/paddle/lib/libpaddle_inference.so
    )
endif()

target_include_directories(paddle_inference INTERFACE ${paddle_SOURCE_DIR}/paddle/include)
install(IMPORTED_RUNTIME_ARTIFACTS paddle_inference)
add_library(paddle::inference ALIAS paddle_inference)

if(WIN32)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mkldnn/lib/mkldnn.dll DESTINATION bin)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/mklml.dll DESTINATION bin)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/libiomp5md.dll DESTINATION bin)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/paddle2onnx/lib/paddle2onnx.dll DESTINATION bin)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/onnxruntime/lib/onnxruntime.dll DESTINATION bin)
elseif(APPLE)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mkldnn/lib/libmkldnn.dylib DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/libmklml.dylib DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/libiomp5.dylib DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/paddle2onnx/lib/libpaddle2onnx.dylib DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/onnxruntime/lib/libonnxruntime.dylib DESTINATION lib)
else()
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mkldnn/lib/libmkldnn.so DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/libmklml.so DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/mklml/lib/libiomp5.so DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/paddle2onnx/lib/libpaddle2onnx.so DESTINATION lib)
    install(FILES ${paddle_SOURCE_DIR}/third_party/install/onnxruntime/lib/libonnxruntime.so DESTINATION lib)
endif()
