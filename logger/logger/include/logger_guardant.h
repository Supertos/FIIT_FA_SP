#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_LOGGER_GUARDANT_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_LOGGER_GUARDANT_H

#include "logger.h"

class logger_guardant
{

public:

    virtual ~logger_guardant() noexcept = default;

public:

    logger_guardant & log_with_guard(
        std::string const &message,
        logger::severity severity) &;

    logger_guardant &trace_with_guard(
        std::string const &message) &;

    logger_guardant &debug_with_guard(
        std::string const &message) &;

    logger_guardant &information_with_guard(
        std::string const &message) &;

    logger_guardant &warning_with_guard(
        std::string const &message) &;

    logger_guardant &error_with_guard(
        std::string const &message) &;

    logger_guardant &critical_with_guard(
        std::string const &message) &;

protected:

    inline virtual logger *get_logger() const = 0;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_LOGGER_GUARDANT_H