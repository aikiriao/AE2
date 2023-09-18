if (BUILD_AE2_DOCUMENTATION)

FIND_PACKAGE(Doxygen)

if (DOXYGEN_FOUND)
        set(DOXYGEN_LANGUAGE "English" CACHE STRING "Language used by doxygen")
        mark_as_advanced(DOXYGEN_LANGUAGE)
        
        # img以下の図も参照する
        set(CMAKE_DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/img")
        
        # Doxygenに入力するディレクトリリスト(;区切り)
        set(DOXYGEN_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libs;${CMAKE_SOURCE_DIR}/plugins;")
        
        # Doxygenの@CMAKE_DOXYGEN_INPUT_LIST@に設定
        set(DOXYGEN_OUTPUT_DIR html)
        string(REGEX REPLACE ";" " " CMAKE_DOXYGEN_INPUT_LIST "${DOXYGEN_SOURCE_DIR}")
        
        configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        set(HTML_TARGET "html")
        
        add_custom_target(${HTML_TARGET} ALL
                ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
else (DOXYGEN_FOUND)
        message(FATAL_ERROR "doxygen couldn't be found")
endif (DOXYGEN_FOUND)

endif (BUILD_AE2_DOCUMENTATION)
