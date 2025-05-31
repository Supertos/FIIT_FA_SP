#include <filesystem>
#include <utility>
#include <not_implemented.h>
#include "../include/client_logger_builder.h"
#include <algorithm>
#include <cctype>
#include <fstream>


using namespace nlohmann;

logger_builder& client_logger_builder::add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) & {

    auto streams = _output_streams.find(severity);

    if (streams == _output_streams.end()) {
		streams = _output_streams.emplace(
            severity,
            std::make_pair(
                std::forward_list<client_logger::refcounted_stream>(),
                false
            )
        ).first;
    }

    auto &list = streams->second.first;
    std::string canonical = std::filesystem::weakly_canonical(
        stream_file_path
    ).string();

    bool found = false;
    for (auto const &rcs : list) {
        if (rcs._stream.first == canonical) {
            found = true;
            break;
        }
    }

    if (!found) {
        list.emplace_front(canonical);
    }

	return *this;
}

logger_builder& client_logger_builder::add_console_stream(
        logger::severity severity) & {

    auto streams = _output_streams.find(severity);
    if (streams == _output_streams.end()) {
		streams = _output_streams.emplace(
            severity,
            std::make_pair(
                std::forward_list<client_logger::refcounted_stream>(),
                false
            )
        ).first;
    }
    streams->second.second = true;
	return *this;
}

logger_builder& client_logger_builder::transform_with_configuration(
        std::string const &configuration_file_path,
        std::string const &configuration_path) & {

    std::ifstream file(configuration_file_path);

	if (!file.is_open()) {
        throw std::ios_base::failure(
            "Can't open file " + configuration_file_path
        );
    }

	json data = json::parse(file);
	file.close();
    
    auto json_ptr = json::json_pointer("/" + configuration_path);
    if (!data.contains(json_ptr)) {
        throw std::ios_base::failure(
            "Can't find '" + configuration_path 
            + "' in '" + configuration_file_path + "'"
        );
    }

    auto& opened_json = data[json_ptr];

    for (auto& [key, value] : opened_json.items()) {
        if (key == "format") {
            if (value.is_string()) {
                set_format(value.get<std::string>());
            }
        }
        try {

            std::string upper_key = key;
            std::transform(
                upper_key.begin(),
                upper_key.end(),
                upper_key.begin(),
                [](unsigned char c) {
                    return std::toupper(c);
                }
            );
            
            logger::severity sev = logger_builder::string_to_severity(
                upper_key
            );
            parse_severity(sev, value);
        } 
        catch (const std::out_of_range&) {
        }
    }

	return *this;
}

logger_builder& client_logger_builder::clear() & {
    _output_streams.clear();
    _format = "%m";
    return *this;
}

logger *client_logger_builder::build() const {
    return new client_logger(_output_streams, _format);
}

logger_builder& client_logger_builder::set_format(
        const std::string &format) & {
    // add some check?
    _format = format;
    return *this;
}

// write path + severity from JSON to _output_streams
void client_logger_builder::parse_severity(
        logger::severity sev,
        nlohmann::json& j) {
    
	if (j.empty() || !j.is_object())
        return;
    
	auto data_paths = j.find("paths");
	if (data_paths != j.end() && data_paths->is_array()) {
		json data = *data_paths;
		for (const json& js: data) {
			if (js.empty() || !js.is_string()) continue;

			const std::string& path = js;
            add_file_stream(path, sev);
		}
	}

	auto console = j.find("console");
	if (console != j.end() && console->is_boolean()) {
        if (console->get<bool>())
            add_console_stream(sev);
	}
}

// Useless for client logger
logger_builder& client_logger_builder::set_destination(
        const std::string &format) & {
    return *this;
}
