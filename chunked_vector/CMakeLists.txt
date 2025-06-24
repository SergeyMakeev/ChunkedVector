set(HEADERS
    chunked_vector.h
    )

add_library(chunked_vector INTERFACE)
target_include_directories(chunked_vector INTERFACE ./)
target_compile_features(chunked_vector INTERFACE cxx_std_17)

