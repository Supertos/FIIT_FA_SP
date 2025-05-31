add_subdirectory(tests)

add_library(
        mp_os_arthmtc_bg_intgr
        include/big_int.h
        src/big_int.cpp)

target_include_directories(
        mp_os_arthmtc_bg_intgr
        PUBLIC
        ./include)

target_link_libraries(
        mp_os_arthmtc_bg_intgr
        PUBLIC
        mp_os_cmmn)
target_link_libraries(
        mp_os_arthmtc_bg_intgr
        PUBLIC
        mp_os_allctr_allctr)