cmake_minimum_required(VERSION 3.15)

set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# プラグイン名
set(PLUGIN_NAME AE2Distortion)

# プロジェクト名
project(${PLUGIN_NAME}
    LANGUAGES CXX
    VERSION 0.0.1
    )

# ライブラリのテストはしない
set(without-test 1)

# 依存するサブディレクトリを追加
add_subdirectory(${PROJECT_ROOT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/libae2)

# プラグイン追加
juce_add_plugin(${PLUGIN_NAME}
    COMPANY_NAME "aikiriao" # 会社名
    COMPANY_COPYRIGHT "aikiriao" # コピーライト
    COMPANY_WEBSITE "https://github.com/aikiriao/" # 会社Web
    COMPANY_EMAIL "samuraiaiki@gmail.com" # メールアドレス
    IS_SYNTH FALSE # 音源生成か否か
    PLUGIN_MANUFACTURER_CODE CCCC # 最低一文字大文字でプラグインベンダーのIDを指定
    PLUGIN_CODE XXXX # 最低一文字大文字でプラグインのユニークIDを指定
    FORMATS Standalone AU VST3 # ビルド対象のプラグイン
    VST3_CATEGORIES "Fx" # VSTのカテゴリ文字列
    AU_MAIN_TYPE "kAudioUnitType_Effect" # AudioUnitのタイプ
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This applicaion requires a permission to use an audio input device of your computer. By Default, Built-In microphone will be used."
    PRODUCT_NAME ${PLUGIN_NAME} # プラグイン名文字列
    )

# ソースディレクトリ
add_subdirectory(src)

# インクルードパス
target_include_directories(${PLUGIN_NAME}
    PRIVATE
    ${PROJECT_ROOT_PATH}/include
    )

# リンクするライブラリ
target_link_libraries(${PLUGIN_NAME}
    PRIVATE
    ae2
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_cryptography
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_opengl
    juce::juce_product_unlocking
    )

# JuceHeader.hの自動生成
juce_generate_juce_header(${PLUGIN_NAME})

# インクルードパス
target_include_directories(${PLUGIN_NAME}
    PRIVATE
    ${PROJECT_ROOT_PATH}/include
    )

# コンパイルオプション
target_compile_definitions(${PLUGIN_NAME}
    PUBLIC 
    JUCE_VST3_CAN_REPLACE_VST2=0 # VST2には対応していない（JUCE SDKにVST2.4 SDKが含まれていないため）
    JUCE_WEB_BROWSER=0 # TODO: gtkの環境を準備するのが困難だったため
    )

# コンパイルオプション
if(MSVC)
    target_compile_options(${PLUGIN_NAME} PRIVATE /W4)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D DEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /D NDEBUG")
else()
    target_compile_options(${PLUGIN_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wformat=2 -Wstrict-aliasing=2 -Wconversion -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
