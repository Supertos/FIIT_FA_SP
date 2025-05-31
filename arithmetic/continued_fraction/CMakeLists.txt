add_subdirectory(tests)

add_library(
        mp_os_arthmtc_cntnd_frctn
        src/continued_fraction.cpp)

target_include_directories(
        mp_os_arthmtc_cntnd_frctn
        PUBLIC
        ./include)

target_link_libraries(
        mp_os_arthmtc_cntnd_frctn
        PUBLIC
        mp_os_allctr_allctr)
target_link_libraries(
        mp_os_arthmtc_cntnd_frctn
        PUBLIC
        mp_os_arthmtc_frctn)