add_library(
        mp_os_lggr_lggr
        src/logger.cpp
        src/logger_builder.cpp
        src/logger_guardant.cpp)

target_include_directories(
        mp_os_lggr_lggr
        PUBLIC
        ./include)