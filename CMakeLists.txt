cmake_minimum_required(VERSION 3.15)

set(AE2_VERSION "0.0.1")

option(BUILD_AE2_DOCUMENTATION "Create doxygen documentation for developers" ON)

# 静的ライブラリ
project(AE2 C)
set(LIB_NAME ae2)
add_library(${LIB_NAME}
    STATIC
    $<TARGET_OBJECTS:ae2_ring_buffer>
    $<TARGET_OBJECTS:ae2_fft>
    $<TARGET_OBJECTS:ae2_window_function>
    $<TARGET_OBJECTS:ae2_iir_filter>
    $<TARGET_OBJECTS:ae2_convolve>
    )

# 依存するプロジェクト
add_subdirectory(libs)
add_subdirectory(doc)

# テスト
if(NOT without-test)
    enable_testing()
    # C++環境でないとgtestがビルドできないので、CXXプロジェクトを作る
    # -> Cとの挙動で差異が生じるかもしれない...
    project(AE2Test CXX)
    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D DEBUG")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /D NDEBUG")
    else()
        set(CMAKE_CXX_FLAGS "-std=gnu++11") # gtestがGNU独自拡張を使用しているため
        set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    endif()
    include(cmake/gtest.cmake)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_subdirectory(test)
endif()
