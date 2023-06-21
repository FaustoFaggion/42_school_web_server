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

typedef struct client {
	int				fd;
	time_t			start_connection;
	std::string		response;
} t_client;

class WebServ
{
	private:
		int										_fd_listener;
		ListenerSocket							_listener;
		std::map<std::string, directive>		locations;
		std::vector<std::string>				_indexes;

		std::map<int, t_client>		map_connections;
		int 						_efd;
		int							_nfds;
		struct epoll_event			_ev;
		struct epoll_event			_ep_event [MAX_CONNECTIONS];
		socklen_t 					_addrlen;
		struct sockaddr_storage		_client_saddr; // Can store a IPv4 and IPv6 struct

	public:
		WebServ();
		WebServ(FileParser file);
		~WebServ();
	
		int				getFdListener() const;
		ListenerSocket	getListener() const;

		void	setup_server(int type);
		void	create_listener_socket();
		void	create_connections();
		void	run();
		
		void	delete_timeout_socket();
		void	accept_new_connection();
		void	receive_data(int i);
		void	response(int i);

		void			response_parser(std::string &file);
		void			request_parser(std::string request, std::string &method, std::string &path, std::string &protocol);
		std::string		looking_for_path(std::string path);
		void			chk_indexies(std::string path, std::string &html);
		void			diretory_list(std::stringstream &buff, std::string path, std::string html);
		void			buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html);
		void			http_response_syntax(std::string status, std::string &request, std::stringstream &buff);
};

#endif