add_executable(
        mp_os_arthmtc_bg_intgr_tests_bgnt
        big_integer_tests.cpp)

target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_bgnt
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_bgnt
        PRIVATE
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_bgnt
        PRIVATE
        mp_os_arthmtc_bg_intgr)