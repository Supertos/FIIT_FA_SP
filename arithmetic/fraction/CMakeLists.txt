add_subdirectory(tests)

add_library(
        mp_os_arthmtc_frctn
        src/fraction.cpp)

target_include_directories(
        mp_os_arthmtc_frctn
        PUBLIC
        ./include)

target_link_libraries(
        mp_os_arthmtc_frctn
        PUBLIC
        mp_os_allctr_allctr)
target_link_libraries(
        mp_os_arthmtc_frctn
        PUBLIC
        mp_os_arthmtc_bg_intgr)