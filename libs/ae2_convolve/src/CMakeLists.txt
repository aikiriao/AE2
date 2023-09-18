target_sources(${LIB_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ae2_fft_convolve.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ae2_karatsuba.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ae2_zerolatency_fft_convolve.c
    )
