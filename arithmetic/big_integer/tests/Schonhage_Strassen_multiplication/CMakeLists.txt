add_executable(
        mp_os_arthmtc_bg_intgr_tests_Schonhage_Strassen
        Schonhage_Strassen_multiplication_tests.cpp)

target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Schonhage_Strassen
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Schonhage_Strassen
        PRIVATE
        mp_os_lggr_clnt_lggr)
target_link_libraries(
        mp_os_arthmtc_bg_intgr_tests_Schonhage_Strassen
        PRIVATE
        mp_os_arthmtc_bg_intgr)