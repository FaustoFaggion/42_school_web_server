#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include "ListenerSocket.hpp"
#include "FileParser.hpp"
#include <map>

class WebServ
{
	private:
		FileParser			_server_config_file;
		/*LISTENER*/
		int					_domain;
		int					_type;
		int					_flag;
		std::string			_server_name;
		std::string			_port;
		int					_fd_listener;

		std::map<std::string, directive>	_location;
		std::vector<std::string>			_webserv_index;
		size_t								_buff_size;
		size_t								_body_max_size;
		std::map<std::string, std::string>	_error_page_map;
		std::vector<std::string>			_allowed_methods;

		struct addrinfo						_hints;
		struct addrinfo						*_result;
		int									_s;

	public:
		WebServ();
		WebServ(FileParser fileParser);
		~WebServ();
	
		int									getFdListener() const;
		std::string							get_port() const;
		std::map<std::string, directive>	getLocation() const;
		std::vector<std::string> 			getWebservIndex() const;
		size_t								getBuffSize() const;
		size_t								getBodyMaxSize() const;
		std::vector<std::string>			getAllowedMethods() const;
		std::map<std::string, std::string>	getErrorPageMap() const;
		FileParser							getServerConfigFile() const;
		std::string							getServerName() const;

		int		setupServers(int max_worker_connections);
		int		setup_addrinfo();
		void	create_listener_socket();
		void	bind_listener_socket();
		void	start_listen();


		void	setup_domain_and_port(std::vector<std::string> listens, int &domain, std::string &port);
		void	setup_server_name_flag(std::string buff, int &flag);
		void	setup_error_pages();

		void	parseListenLine(size_t i, std::string &line, std::string &response);
};

#endif