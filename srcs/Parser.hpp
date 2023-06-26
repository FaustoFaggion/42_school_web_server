#ifndef PARSER_HPP
#define PARSER_HPP

#include "FileParser.hpp"

class Parser : public FileParser
{
		std::string							_server_conf_file;
		listener_socket						_listener;
		std::map<std::string, directive>	_path;
		std::vector<std::string>			_index;
};

#endif