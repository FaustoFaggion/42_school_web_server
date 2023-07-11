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
#include "HttpResponse.hpp"
#include <map>

class WebServ : protected FileParser, ListenerSocket, HttpRequest, HttpResponse
{
	private:
		int							_fd_listener;

		std::map<int, t_client>		map_connections;
		int 						_efd;
		int							_nfds;
		struct epoll_event			_ev;
		struct epoll_event			_ep_event [MAX_CONNECTIONS];
		socklen_t 					_addrlen;
		struct sockaddr_storage		_client_saddr; // Can store a IPv4 and IPv6 struct

	public:
		WebServ();
		WebServ(char *file, std::string server_name);
		~WebServ();
	
		int				getFdListener() const;

		void	parse_file(char *file, std::string server_name);
		void	setup_server(int type);
		void	create_listener_socket();
		void	create_connections();
		void	run();
		
		void	delete_timeout_socket();
		int		accept_new_connection();
		void	receive_data(int i);
		void	verify_received_data(t_client &client, std::map<std::string, directive> &locations, std::vector<std::string> indexes, std::string buff);
		void	decide_how_to_respond(t_client &client, std::map<std::string, directive> &locations, int i);
		void	write_data_to_cgi(t_client &client, char *buff, size_t numbytes);
		void	write_response_to_string(t_client &client);
		void	send_response(int i);

		void	initialize_client_struct(std::map<int, t_client> &map, int fd_new);
		void	exec_cgi(std::string &html, t_client &client);
};

#endif