cmake_minimum_required(VERSION 3.10)

project(PaddleOCR_json VERSION 1.2.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/installed)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

find_path(DIRENT_INCLUDE_DIRS "dirent.h")

file(GLOB PPOCR_SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${PPOCR_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${DIRENT_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE GOOGLE_GLOG_IS_A_DLL)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /openmp)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    paddle::inference
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
    gflags::gflags
    autolog::header
    glog::glog
)

if (APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

install(TARGETS ${PROJECT_NAME})
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
install(IMPORTED_RUNTIME_ARTIFACTS
    ${OpenCV_LIBS}
    glog::glog
    gflags::gflags
)

install(CODE [[
file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
    UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
    LIBRARIES $<TARGET_FILE:${PROJECT_NAME}>
    DIRECTORIES $<TARGET_FILE_DIR:${PROJECT_NAME}>
    PRE_INCLUDE_REGEXES $<TARGET_FILE_DIR:${PROJECT_NAME}>
    PRE_EXCLUDE_REGEXES "system32"
    POST_INCLUDE_REGEXES $<TARGET_FILE_DIR:${PROJECT_NAME}>
    POST_EXCLUDE_REGEXES "system32"
)
foreach(DEP_LIB ${RESOLVED_DEPS})
    file(INSTALL ${DEP_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endforeach()
]])
