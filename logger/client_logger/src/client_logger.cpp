#include <string>
#include <sstream>
#include <algorithm>
#include <utility>
#include "../include/client_logger.h"
#include <not_implemented.h>


std::unordered_map<std::string, std::pair<size_t, std::ofstream>>
client_logger::refcounted_stream::_global_streams;

logger& client_logger::log(
        const std::string &text,
        logger::severity severity) & {
    
    std::string output = make_format(text, severity);
    auto log_streams = _output_streams.find(severity);
    if (log_streams == _output_streams.end())
        return *this;

    // console output
    if (log_streams->second.second)
        std::cout << output << std::endl;

    // file stream
    for (auto &out_stream: log_streams->second.first) {
        if (out_stream._stream.second != nullptr)
            *out_stream._stream.second << output << std::endl;
    }

    return *this;
}

std::string client_logger::make_format(
        const std::string &message,
        severity sev) const {
    
	std::ostringstream oss;
	for (auto elem = _format.begin(), end = _format.end();
            elem != end; ++elem) {

		flag type = flag::NO_FLAG;
		if (*elem == '%') {
            type = char_to_flag(*(elem + 1));
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
			++elem;  // ignore %'invalid letter'
        }
        else
            oss << *elem;
	}
	return oss.str();
}

client_logger::client_logger(
        const std::unordered_map<
            logger::severity,
            std::pair<std::forward_list<refcounted_stream>, bool>
        > &streams,
        std::string format
    ): _format(std::move(format)), _output_streams(streams) {
}

client_logger::flag client_logger::char_to_flag(char c) noexcept {
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

client_logger::client_logger(const client_logger &other)
        :_output_streams(other._output_streams),
        _format(other._format) {
}

client_logger &client_logger::operator=(const client_logger &other) {
	if (this != &other) {
		_output_streams = other._output_streams;
		_format = other._format;
	}
	return *this;
}

client_logger::client_logger(client_logger &&other) noexcept
        : _output_streams(std::move(other._output_streams)),
        _format(std::move(other._format)) {
}

client_logger &client_logger::operator=(client_logger &&other) noexcept {
	if (this != &other) {
		_output_streams = std::move(other._output_streams);
		_format = std::move(other._format);
	}
	return *this;
}

client_logger::~client_logger() noexcept =default;

client_logger::refcounted_stream::refcounted_stream(const std::string &path) {
	auto opened_stream = _global_streams.find(path);

    if (opened_stream != _global_streams.end()) {
		++opened_stream->second.first;
		_stream = std::make_pair(path, &opened_stream->second.second);
    }
    else {
        auto inserted_stream = _global_streams.emplace(
            path, 
            std::pair<size_t, std::ofstream>(1, std::ofstream(path))
        );
        if (!inserted_stream.second ||
                !inserted_stream.first->second.second.is_open()) {

            if (inserted_stream.second)
                _global_streams.erase(inserted_stream.first);

            throw std::ios_base::failure(
                "File " + path + " could not be opened"
            );
        }
        _stream = std::make_pair(path, &inserted_stream.first->second.second);
    }
}

client_logger::refcounted_stream::refcounted_stream(
        const client_logger::refcounted_stream &oth) {

	auto opened_stream = _global_streams.find(oth._stream.first);

	if (opened_stream != _global_streams.end()) {
		++opened_stream->second.first;
		_stream = std::make_pair(
            opened_stream->first,
            &opened_stream->second.second
        );
	}
    else throw std::out_of_range(
        "File path was lost in _global_streams????");  // delete it 
}

client_logger::refcounted_stream &
client_logger::refcounted_stream::operator=(
        const client_logger::refcounted_stream &oth) {

    if (this != &oth){
    	auto opened_stream = _global_streams.find(oth._stream.first);
        ++opened_stream->second.first;
        // _stream = oth._stream;  // oth._stream.second could be nullptr;
		_stream = std::make_pair(
            opened_stream->first,
            &opened_stream->second.second
        );
    }
    return *this;
}

client_logger::refcounted_stream::refcounted_stream(
        client_logger::refcounted_stream &&oth
        ) noexcept : _stream(std::move(oth._stream)) {
	oth._stream.second = nullptr;
}


client_logger::refcounted_stream &client_logger::refcounted_stream::operator=(
        client_logger::refcounted_stream &&oth) noexcept {
    if (this != &oth){
        _stream = std::move(oth._stream);
		oth._stream.second = nullptr;
    }
    return *this;
}

client_logger::refcounted_stream::~refcounted_stream() {
	if (_stream.second != nullptr) {
		auto opened_stream = _global_streams.find(_stream.first);
		--opened_stream->second.first;
		if (opened_stream->second.first == 0) {
			opened_stream->second.second.close();
			_global_streams.erase(opened_stream);
		}
	}
}
