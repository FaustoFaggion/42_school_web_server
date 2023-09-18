#ifndef FileParser_HPP
#define FileParser_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include "ListenerSocket.hpp"
#include "ServerExceptions.hpp"

#define SIMPLE_DIRECTIVE	0
#define BLOCK_DIRECTIVE		1

class FileParser {

	private:
		std::vector<std::string>			listen;
		std::string							serverName;
		std::map<std::string, directive>	locations;
		std::vector<std::string>			errorPath;
		std::vector<std::string>			simpleIndexes;
		size_t								bufferSize;
		size_t								bodySize;

	public:
		FileParser();
		~FileParser();

		//utils
		int						parse_path(std::string &str, std::string find, std::string &root_path, int flag);
		static void				count_brackets(size_t bk_close, size_t bk_open, size_t it, size_t size, std::string &servers_config_file);
		void					atoi_bytes(std::string buff, size_t &param_size, std::string error_msg);
		static std::string		str_substring(std::string &str, std::string find, int init, char finish);
		static int				chk_simple_directive(std::string &str);
		std::string				get_simple_directive_value(std::string &str, int &value_count);
		std::string				get_block_directive_value(std::string &str, int &value_count);

		//parse_head_methods
		static void				parseHead(char *file, std::string &_servers_config_file, std::string &_events, int &_max_worker_connections, std::vector<std::string> &_servers);
		static void				parse_file_to_string(char *file, std::string &response);
		static void				parse_events_from_config_file(std::string &servers_config_file, std::string &events);
		static void				parse_worker_connections(std::string &events, int &worker_connections);
		static void				setup_worker_connections(std::string buff, int &worker_connections);
		static void				parse_work_processes(std::string &servers_config_file, int &max_worker_connections);
		static void				parse_server_from_config_file(std::string &servers_config_file, std::vector<std::string> &servers);
		
		//parse_serverBlock_methods
		static FileParser		parseServersBlock(std::string &server);
		void					parse_listening(std::string &server_conf_file);
		void					parse_server_name_flag(std::string &server_conf_file);
		void					parse_simple_root_directive(std::string &server_config_file);
		void					parse_buffer_size(std::string &servers_config_file);
		void					parse_body_size(std::string &servers_config_file);
		void					parse_error_page(std::string &servers_config_file);
		void					parse_index_simple_directive(std::string &server_config_file);
		void					parse_locations(std::string &servers_config_file);
		void					parse_location_block(std::string &servers_config_file, std::string &server_path, std::string &location, std::string &root_path, std::string	&request_path);
		void					parse_request_absolute_path_in_location_block(std::string &server_path, std::string &root_path, std::string &request_path);
		void					parse_autoindex_in_location_block(std::string &request_path, std::string &location);
		void					parse_index_in_location_block(std::string &location, std::string &request_path);
		void					parse_index_block_directive(std::vector<std::string> &idx, std::string &str);
		void					parse_allowed_methods_in_location_block(std::string &location, std::string &request_path);
		void					parse_allowed_methods(std::string &directive, std::vector<std::string> &allowed_methods);
		void					parse_redirects_in_location_block(std::string &location, std::string &request_path);
		void					parse_cgi_pass_in_location_block(std::string &location, std::string &request_path);


		//getters
		std::vector<std::string>			getListen() const;
		std::string							getServerName() const;
		std::map<std::string, directive>	getLocations() const;
		size_t								getBufferSize() const;
		size_t								getBodySize() const;
		std::vector<std::string>			getErrorPath() const;
		std::vector<std::string>			getSimpleIndexes() const;
		
};

#endif