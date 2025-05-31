add_executable(
        mp_os_lggr_clnt_lggr_tests
        client_logger_tests.cpp)

target_link_libraries(
        mp_os_lggr_clnt_lggr_tests
        PRIVATE
        gtest_main)
target_link_libraries(
        mp_os_lggr_clnt_lggr_tests
        PUBLIC
        mp_os_lggr_clnt_lggr)