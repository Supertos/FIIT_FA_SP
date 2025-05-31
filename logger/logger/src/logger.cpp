#include "../include/logger.h"
#include <iomanip>
#include <sstream>

logger & logger::trace(
    std::string const &message) &
{
    return log(message, logger::severity::trace);
}

logger &logger::debug(
    std::string const &message) &
{
    return log(message, logger::severity::debug);
}

logger &logger::information(
    std::string const &message) &
{
    return log(message, logger::severity::information);
}

logger &logger::warning(
    std::string const &message) &
{
    return log(message, logger::severity::warning);
}

logger & logger::error(
    std::string const &message) &
{
    return log(message, logger::severity::error);
}

logger &logger::critical(
    std::string const &message) &
{
    return log(message, logger::severity::critical);
}

std::string logger::severity_to_string(
    logger::severity severity)
{
    switch (severity)
    {
        case logger::severity::trace:
            return "TRACE";
        case logger::severity::debug:
            return "DEBUG";
        case logger::severity::information:
            return "INFORMATION";
        case logger::severity::warning:
            return "WARNING";
        case logger::severity::error:
            return "ERROR";
        case logger::severity::critical:
            return "CRITICAL";
    }

    throw std::out_of_range("Invalid severity value");
}

std::string logger::current_datetime_to_string()
{
    auto time = std::time(nullptr);

    std::ostringstream result_stream;
    result_stream << std::put_time(std::localtime(&time), "%d.%m.%Y %H:%M:%S");

    return result_stream.str();
}

std::string logger::current_date_to_string()
{
    auto time = std::time(nullptr);

    std::ostringstream result_stream;
    result_stream << std::put_time(std::localtime(&time), "%d.%m.%Y");

    return result_stream.str();
}

std::string logger::current_time_to_string()
{
    auto time = std::time(nullptr);

    std::ostringstream result_stream;
    result_stream << std::put_time(std::localtime(&time), "%H:%M:%S");

    return result_stream.str();
}
