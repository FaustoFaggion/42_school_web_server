#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "ListenerSocket.hpp"

class WebServ
{
	private:
		int							_domain;
		int							_type;
		int							_flag;
		std::string					_port;
		int							_max_connections;
		int							_fd_listener;
		ListenerSocket				_listener;
		int 						_efd;
		int							_nfds;
		struct epoll_event			_ev;
		struct epoll_event			_ep_event [MAX_CONNECTIONS];
		socklen_t 					_addrlen;
		struct sockaddr_storage		_client_saddr; // Can store a IPv4 and IPv6 struct
	
	public:
		WebServ();
		WebServ(int type);
		~WebServ();
	
		int				getFdListener() const;
		ListenerSocket	getListener() const;

		void	parse_file(char *file);
		void	setup_server(int type);
		void	create_listener_socket();
		void	create_connections();
		void	run();
	private:
		void	fill_struct_conf_file(std::string buff);
};

#endif