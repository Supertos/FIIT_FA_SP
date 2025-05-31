add_executable(
        mp_os_arthmtc_bg_intgr_tests_trvl_dvsn
        trivial_division_tests.cpp)

target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_dvsn
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_dvsn
        PRIVATE
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_trvl_dvsn
        PRIVATE
        mp_os_arthmtc_bg_intgr)