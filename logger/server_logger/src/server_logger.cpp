#include <not_implemented.h>
#include <httplib.h>
#include <ranges>
#include "../include/server_logger.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

const std::string server_logger::_separator = ","; 

server_logger::~server_logger() noexcept {
    httplib::Params params;
    params.emplace("pid", std::to_string(inner_getpid()));
    _client.Get("/destroy", params, httplib::Headers());
}

logger& server_logger::log(
    const std::string &text,
    logger::severity severity) & {

        httplib::Params params;
    params.emplace("pid", std::to_string(server_logger::inner_getpid()));
    params.emplace("sev", severity_to_string(severity));
    params.emplace("message", make_format(text, severity));
	auto res = _client.Get("/log", params, httplib::Headers());
	return *this;
}

std::string server_logger::make_format(
        const std::string &message, severity sev
    ) const {

	std::ostringstream oss;
	for (auto elem = _format.begin(), end = _format.end();
            elem != end; ++elem) {

		flag type = flag::NO_FLAG;
		if (*elem == '%') {
            ++elem;
            type = char_to_flag(*elem);
			switch (type) {
				case flag::DATE:
					oss << current_date_to_string();
					break;
				case flag::TIME:
					oss << current_time_to_string();
					break;
				case flag::SEVERITY:
					oss << severity_to_string(sev);
					break;
                case flag::MESSAGE:
                    oss << message;
					break;
				default:
					break;
			}
        }
        else
            oss << *elem;
	}
	return oss.str();
}

server_logger::flag server_logger::char_to_flag(char c) noexcept {
	switch (c) {
		case 'd':
			return flag::DATE;
		case 't':
			return flag::TIME;
		case 's':
			return flag::SEVERITY;
		case 'm':
			return flag::MESSAGE;
		default:
			return flag::NO_FLAG;
	}
}

server_logger::server_logger(const std::string& dest,
        const std::unordered_map<logger::severity, std::pair<std::string, bool>> &streams,
        std::string format
    ): _client(dest), _format(std::move(format)), _streams(streams){

	std::string pid = std::to_string(inner_getpid());
	auto res = _client.Get("/destroy?pid=" + pid);

    for (auto& pair : streams) {

        httplib::Params params;
        params.emplace("pid", std::to_string(server_logger::inner_getpid()));
        params.emplace("sev", severity_to_string(pair.first));
        params.emplace("console", pair.second.second ? "1" : "0");
        params.emplace("path", pair.second.first);

        std::string url = "/init";
        auto res = _client.Get(url, params, httplib::Headers());
    }

}

int server_logger::inner_getpid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return getpid();
#endif
}

server_logger::server_logger(const server_logger &other)
        :_client(other._client.host(), other._client.port()),
        _format(other._format), _streams(other._streams) {
    
    for (auto& pair : _streams) {
        httplib::Params par;
        par.emplace("pid", std::to_string(server_logger::inner_getpid()));
        par.emplace("sev", severity_to_string(pair.first));

        std::string paths = pair.second.first;
        for (auto token : paths | std::views::split(_separator)) {
            if (!std::string(std::string_view(token)).empty())
                par.emplace(
                    "paths",
                    std::string(std::string_view(token))
                );
        }
        
        par.emplace("console", pair.second.second ? "1" : "0");

        auto res = _client.Get("/init", par, httplib::Headers());
    }
}

server_logger &server_logger::operator=(const server_logger &other) {
    if (this != &other) {
        _client = httplib::Client(other._client.host(), other._client.port());
        _format = other._format; 
        _streams = other._streams;

        for (auto& pair : _streams) {
        httplib::Params par;
        par.emplace("pid", std::to_string(server_logger::inner_getpid()));
        par.emplace("sev", severity_to_string(pair.first));

        std::string paths = pair.second.first;
        for (auto token : paths | std::views::split(_separator)) {
            if (!std::string(std::string_view(token)).empty())
                par.emplace(
                    "paths",
                    std::string(std::string_view(token))
                );
        }
        
        par.emplace("console", pair.second.second ? "1" : "0");

        auto res = _client.Get("/init", par, httplib::Headers());
    }

	}
    return *this;
}

server_logger::server_logger(server_logger &&other) noexcept
        :_client(std::move(other._client)), _format(std::move(other._format)),
        _streams(std::move(other._streams)) {

    other._streams = std::unordered_map<logger::severity, std::pair<std::string, bool>>();
    other._client = httplib::Client("http://127.0.0.1:9200");
}

server_logger &server_logger::operator=(server_logger &&other) noexcept{

    _client = std::move(other._client);
    _format = std::move(other._format);
    _streams = std::move(other._streams);

    other._streams = std::unordered_map<logger::severity, std::pair<std::string, bool>>();
    other._client = httplib::Client("http://127.0.0.1:9200");

    return *this;
}
