add_executable(
        mp_os_arthmtc_bg_intgr_tests_Burnikel_Ziegler_dvsn
        Burnikel_Ziegler_division_tests.cpp)

target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Burnikel_Ziegler_dvsn
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Burnikel_Ziegler_dvsn
        PRIVATE
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Burnikel_Ziegler_dvsn
        PRIVATE
        mp_os_arthmtc_bg_intgr)