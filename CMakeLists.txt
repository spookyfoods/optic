cmake_minimum_required(VERSION 3.10)
project(WebPPMStudio)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_BINARY_DIR} src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(STB_IMAGE_LOC "${CMAKE_BINARY_DIR}/stb_image.h")
if(NOT EXISTS "${STB_IMAGE_LOC}")
    message(STATUS "Downloading stb_image.h...")
    file(DOWNLOAD 
         https://raw.githubusercontent.com/nothings/stb/master/stb_image.h 
         "${STB_IMAGE_LOC}")
endif()

if(EMSCRIPTEN)
    message("Building for wasm")
    add_executable(ppm_web src/ImageProcessor.cpp src/web_glue.cpp)
    target_link_options(ppm_web PRIVATE
        "--bind"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME='createModule'"
        "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString', 'HEAPU8']"
        "-sEXPORTED_FUNCTIONS=['_malloc','_free']"
        "-sNO_DISABLE_EXCEPTION_CATCHING"
    )
    set_target_properties(ppm_web PROPERTIES 
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
    )

else()
    message("Building for native")
    add_executable(ppm_cli src/ImageProcessor.cpp src/main.cpp src/Filters.cpp)
endif()


# ==========================================
# EXPERIMENTAL SPIKE TARGETS
# ==========================================

find_package(TIFF REQUIRED)

add_executable(cog_spike src/spike/cog_spike.cpp)
target_link_libraries(cog_spike PRIVATE TIFF::TIFF)

add_executable(engine_test src/spike/engine_test.cpp)
add_executable(blur_engine src/spike/blur_engine.cpp)
add_executable(check_blur src/spike/check_blur.cpp)
