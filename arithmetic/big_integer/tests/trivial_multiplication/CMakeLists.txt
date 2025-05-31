add_executable(
        mp_os_arthmtc_bg_intgr_tests_trvl_mltplctn
        trivial_multiplication_tests.cpp)

target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_mltplctn
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_mltplctn
        PRIVATE
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_mltplctn
        PRIVATE
        mp_os_arthmtc_bg_intgr)