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

class FileParser {

	protected:
		std::string							_server_conf_file;
		std::map<std::string, directive>	_path;
		std::vector<std::string>			_index;

	public:
		FileParser();
		FileParser(char *file, std::string server_name);
		~FileParser();

		std::map<std::string, directive>	getPath() const;
		std::vector<std::string> 			getIndex() const;

		void								parse_server(char *file, std::string server_name);
		void								select_server(std::string file, std::string serv_name);
		void								parse_locations(bool simple_root_directive);
		void								parse_path(std::string &str, std::string find, std::string &root_path, int flag);
		bool								parse_simple_root_directive();
		void								parse_index(std::vector<std::string> &idx, std::string &str);

		void								file_to_string(char *file, std::string &buff);
		std::string							str_substring(std::string &str, std::string find, int init, char finish);
		void								chk_simple_directive(std::string &str);
		std::string							get_simple_directive_value(std::string &str);
};

#endif