#ifndef MONITOR_HPP
#define MONITOR_HPP

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
#include "WebServ.hpp"
#include "ServerExceptions.hpp"


#include <map>

class Monitor
{
	private:

		std::string							_servers_config_file;
		std::vector<std::string>			_servers;
		std::string							_events;
		std::vector<WebServ*>				_web_servers;
		std::map<int, WebServ*>				_map_web_servers;
		int									_max_worker_connections;
		std::map<int, t_client>				_map_connect;
		std::vector<int>					_vector_fds;
		

		int 						_m_efd;
		int							_m_nfds;
		struct epoll_event			_m_ev;
		socklen_t 					_m_addrlen;
		struct sockaddr_storage		_m_client_saddr; // Can store a IPv4 and IPv6 struct
		struct epoll_event			*_m_ep_event;

	public:
		Monitor();
		~Monitor();
	
		int						getFdListener() const;
		std::vector<WebServ*>	getWebServers() const;
		std::vector<int>		getVectorFds() const;
		struct epoll_event		getMev() const;
		int						getMefd() const;

		void		initialize_monitor(char *file);

		int			parse_server_conf_file(char *file);
		void		create_connections();
		void		parse_work_processes();

		void		run();
		void		delete_timeout_socket();
		void		close_fd(int i);
		int			chk_listener(int ep_event_fd);
		void		accept_new_connection(WebServ *server);
		void		initialize_client_struct(WebServ *server, std::map<int, t_client> &map, int fd_new);

		void		receive_data(int i);

		void		verify_received_data(t_client &client, std::string buff);
		void		decide_how_to_respond(t_client &client);
		void		write_body_chunks(t_client &client, char *buff, size_t numbytes);
		void		delete_method(t_client &client);
		void		save_file(t_client &client);
		void		write_first_body_chunk_to_cgi(t_client &client);
		void		write_remaining_body_chunks_to_cgi(t_client &client, char *buff, size_t numbytes);

		void		split_header_from_body(t_client &client, std::string buff);
		void		split_body_from_header(t_client &client, std::string buff);
		void		request_parser(t_client &client);
		std::string	parse_line(std::string &request, std::string start, std::string end);
		void		looking_for_path(t_client &client);
		void		chk_indexies(t_client &client, std::string &html, std::string url);

		void		exec_cgi(std::string &html, t_client &client);
		void		cgi_envs_parser(t_client &client, std::string html);

		void		send_response(int i);
		void		write_response_to_string(t_client &client);
		void		response_parser(t_client &client);
		void		diretory_list(std::stringstream &buff, std::string path, std::string html);
		void		buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html);
		void		http_response_syntax(t_client &client, std::stringstream &buff);
		void		http_response_redirection (t_client &client);
		void		http_response_error(t_client &client);
		void		http_response_save_file(t_client &client);
		void		http_response_delete_file(t_client &client);
};

#endif