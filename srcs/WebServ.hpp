#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "ListenerSocket.hpp"
#include "FileParser.hpp"
#include <map>

class WebServ
{
	private:
		int							_fd_listener;
		ListenerSocket				_listener;

		std::map<int, std::string>	connections;
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

		void	request_parser(std::string &file);
};

#endif