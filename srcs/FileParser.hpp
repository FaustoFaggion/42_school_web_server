#ifndef FileParser_HPP
#define FileParser_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>

#define SIMPLE_DIRECTIVE	0
#define BLOCK_DIRECTIVE		1

struct listener_socket {
	int				_domain;
	std::string		_port;
	int				_flag;
	int				_type;
	int				_worker_processes;
};

class FileParser {

	private:
		std::string							_server_conf_file;
		listener_socket						_listener;
		std::map<std::string, directive>	_path;
		std::vector<std::string>			_index;

	public:
		FileParser();
		FileParser(char *file);
		~FileParser();

		int									get_domain() const;
		std::string							get_port() const;
		int									get_flag() const;
		int									get_worker_processes() const;
		int									get_type() const;
		std::map<std::string, directive>	getPath() const;
		std::vector<std::string> 			getIndex() const;

		void								parse_configuration_file(char *file);
		void								parse_listener();
		void								parse_locations(bool simple_root_directive);
		void								parse_path(std::string &str, std::string find, std::string &root_path, int flag);
		bool									parse_simple_root_directive();
		void								setup_listener(std::string buff);
		void								parse_index(std::vector<std::string> &idx, std::string &str);

		void								file_to_string(char *file, std::string &buff);
		std::string							str_substring(std::string &str, std::string find, int init, char finish);
		void								chk_simple_directive(std::string &str);
		std::string							get_simple_directive_value(std::string &str);
};

#endif