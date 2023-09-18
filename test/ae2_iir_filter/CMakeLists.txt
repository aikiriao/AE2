cmake_minimum_required(VERSION 3.15)

set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# テスト名
set(TEST_NAME ae2_iir_filter_test)

# 実行形式ファイル
add_executable(${TEST_NAME}
        ae2_biquad_filter_test.cpp
        ae2_iir_filter_test.cpp
        main.cpp
        )

# インクルードディレクトリ
include_directories(${PROJECT_ROOT_PATH}/libs/ae2_iir_filter/include)

# リンクするライブラリ
target_link_libraries(${TEST_NAME} gtest gtest_main ae2_fft)
if (NOT MSVC)
target_link_libraries(${TEST_NAME} pthread)
endif()

# コンパイルオプション
set_target_properties(${TEST_NAME}
    PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )

add_test(
    NAME ae2_iir_filter
    COMMAND $<TARGET_FILE:${TEST_NAME}>
    )

# run with: ctest -L lib
set_property(
    TEST ae2_iir_filter
    PROPERTY LABELS lib ae2_iir_filter
    )
