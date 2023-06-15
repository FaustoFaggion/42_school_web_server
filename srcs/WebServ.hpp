#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
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
		int							_fd_listener;
		ListenerSocket				_listener;
		std::map<std::string, std::string>	locations;
		std::vector<std::string>	_indexes;

		std::map<int, t_client>	map_connections;
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

		void			request_parser(std::string &file);
		std::string		looking_for_path(std::string path);
};

#endif