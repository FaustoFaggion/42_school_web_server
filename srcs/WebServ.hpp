#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include "ListenerSocket.hpp"

class WebServ
{
	private:
		int							_domain;
		int							_type;
		int							_flag;
		std::string					_port;
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
		WebServ(int domain, int type, int flag, std::string port);
		~WebServ();
	
		int				getFdListener() const;
		ListenerSocket	getListener() const;

		void	setup_server(int domain, int type, int flag, std::string port);
		void	create_listener_socket();
		void	create_connections();
		void	run();
};

#endif