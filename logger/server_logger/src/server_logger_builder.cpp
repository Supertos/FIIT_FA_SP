#include <not_implemented.h>
#include "../include/server_logger_builder.h"
#include <nlohmann/json.hpp>
#include <fstream> 


using namespace nlohmann;

logger_builder& server_logger_builder::add_file_stream(
        std::string const &stream_file_path,
        logger::severity severity) & {

	auto entry = _output_streams.find(severity);
	if (entry == _output_streams.end()){
		entry = _output_streams.emplace(
            severity,
            std::make_pair(std::string(), false)
        ).first;
	}

    if (!entry->second.first.empty())
        entry->second.first += server_logger::_separator;
	
    entry->second.first += stream_file_path;

	return *this;
}

logger_builder& server_logger_builder::add_console_stream(
        logger::severity severity) & {
    auto streams = _output_streams.find(severity);
    if (streams == _output_streams.end()) {
		streams = _output_streams.emplace(
            severity,
            std::make_pair(
                std::string(),
                false
            )
        ).first;
    }
    streams->second.second = true;
	return *this;
}

logger_builder& server_logger_builder::transform_with_configuration(
    std::string const &configuration_file_path,
    std::string const &configuration_path) & {
    
    std::ifstream file(configuration_file_path);

    if (!file.is_open())
        throw std::ios_base::failure(
            "File " + configuration_file_path + " could not be opened"
        );
    
	json data = nlohmann::json::parse(file);

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
            if (value.is_string())
                set_format(value.get<std::string>());
        }
        else if (key == "streams") {
            for (auto& stream_item : value) { 
                std::string type = stream_item["type"];

                if (type == "file") {
                    if (
                        !stream_item.contains("paths")
                        || !stream_item.contains("severities")
                    ) throw std::runtime_error("Invalid file stream config");
                    

                    auto& paths = stream_item["paths"];
                    if (!paths.is_array())
                        throw std::runtime_error("'paths' must be an array");

                    auto& sevs = stream_item["severities"];
                    std::string sev_str;
                    if (sevs.is_string())
                        std::string sev_str = sevs.get<std::string>();
                    else 
                        throw std::runtime_error(
                            "'severities' must be string or array"
                        );

                    for (auto& path : paths) {
                        std::string path_str = path.get<std::string>();
                        add_file_stream(
                            path_str, 
                            string_to_severity(sev_str)
                        );
                    }
                }
                else if (type == "console") {
                    auto& sevs = stream_item["severities"];
                    std::string sev_str;
                    if (sevs.is_string())
                        std::string sev_str = sevs.get<std::string>();
                    else 
                        throw std::runtime_error(
                            "'severities' must be string or array"
                        );
                    add_console_stream(string_to_severity(sev_str));
                }
            }
        }
    }

	return *this;

}

logger_builder& server_logger_builder::clear() & {
    _destination = "http://127.0.0.1:9200";
    _output_streams.clear();
    return *this;
}

logger *server_logger_builder::build() const {
	return new server_logger(_destination, _output_streams, _format);
}

logger_builder& server_logger_builder::set_destination(
        const std::string& dest) & {
	_destination = dest;
	return *this;
}

logger_builder& server_logger_builder::set_format(
        const std::string &format) & {
    _format = format;
    return *this;
}
