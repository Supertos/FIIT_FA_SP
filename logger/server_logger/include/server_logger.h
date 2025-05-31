#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include <unordered_map>
#include <httplib.h>


class server_logger_builder;
class server_logger final:
    public logger
{
    static const std::string _separator;
	std::string _format;
    httplib::Client _client;
	std::unordered_map<logger::severity, std::pair<std::string, bool>> _streams;
    enum class flag
    { DATE, TIME, SEVERITY, MESSAGE, NO_FLAG };

    server_logger(
        const std::string& dest,
        const std::unordered_map<
                logger::severity,
                std::pair<std::string, bool>
            >& streams,
        std::string format);

    friend server_logger_builder;

    static int inner_getpid();
public:

    server_logger(server_logger const &other);

    server_logger &operator=(server_logger const &other);

    server_logger(server_logger &&other) noexcept;

    server_logger &operator=(server_logger &&other) noexcept;

    ~server_logger() noexcept final;

public:

    [[nodiscard]] logger& log(
        const std::string &message,
        logger::severity severity) & override;

private:
    static server_logger::flag char_to_flag(char c) noexcept;
    std::string make_format(const std::string &message, severity sev) const;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H