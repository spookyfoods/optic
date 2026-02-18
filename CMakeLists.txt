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
    target_compile_options(ppm_web PRIVATE "-pthread")
    target_link_options(ppm_web PRIVATE
        "--bind"
        "-pthread"
        # "-sPROXY_TO_PTHREAD=1"
        "-sPTHREAD_POOL_SIZE=4"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME='createModule'"
        "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString', 'HEAPU8', 'FS']"
        "-sEXPORTED_FUNCTIONS=['_malloc','_free']"
        "-sNO_DISABLE_EXCEPTION_CATCHING"
    )
    set_target_properties(ppm_web PROPERTIES 
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
    )

else()
    message("Building for native (Ferrari Mode)")
    add_executable(ppm_cli src/ImageProcessor.cpp src/main.cpp)

    # Detect compiler and apply "Ferrari" flags
    if(MSVC)
        # Windows (Visual Studio)
        # /O2 = Max Speed
        # /arch:AVX2 = Enable advanced vector instructions (SIMD)
        target_compile_options(ppm_cli PRIVATE /O2 /arch:AVX2)
    else()
        # Linux / macOS (GCC or Clang)
        # -O3 = Aggressive optimization (loop unrolling, inlining)
        # -march=native = Auto-detects your CPU's best SIMD features (AVX, NEON, etc.)
        target_compile_options(ppm_cli PRIVATE "-march=native")
    endif()
endif()
