add_subdirectory(tests)

add_library(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_spl_tr
        src/hhh.cpp
        include/splay_tree.h)

target_include_directories(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_spl_tr
        PUBLIC
        ./include)

target_link_libraries(
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr_spl_tr
        PUBLIC
        mp_os_assctv_cntnr_srch_tr_bnr_srch_tr)