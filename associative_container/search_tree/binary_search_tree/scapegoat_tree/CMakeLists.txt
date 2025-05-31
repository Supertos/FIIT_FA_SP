add_subdirectory(tests)

add_library(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_scpgt_tr
        include/scapegoat_tree.h
        src/hhh.cpp)

target_include_directories(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_scpgt_tr
        PUBLIC
        ./include)

target_link_libraries(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_scpgt_tr
        PUBLIC
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr)