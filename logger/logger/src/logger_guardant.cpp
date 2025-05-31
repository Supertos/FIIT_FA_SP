#include "../include/logger_guardant.h"

logger_guardant &logger_guardant::log_with_guard(
    std::string const &message,
    logger::severity severity) &
{
    logger *got_logger = get_logger();
    if (got_logger != nullptr)
    {
        got_logger->log(message, severity);
    }

    return *this;
}

logger_guardant & logger_guardant::trace_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::trace);
}

logger_guardant &logger_guardant::debug_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::debug);
}

logger_guardant &logger_guardant::information_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::information);
}

logger_guardant &logger_guardant::warning_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::warning);
}

logger_guardant &logger_guardant::error_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::error);
}

logger_guardant &logger_guardant::critical_with_guard(
    std::string const &message) &
{
    return log_with_guard(message, logger::severity::critical);
}