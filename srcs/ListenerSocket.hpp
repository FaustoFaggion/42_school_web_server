#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP


#include "main.hpp"

class ListenerSocket
{
	private:
		int					_domain;
		int					_type;
		int					_flag;
		std::string			_port;
		int					_worker_connections;

		struct addrinfo		_hints;
		struct addrinfo		*_result;
		int					_s;
		int					_fd_listener;

	public:
		ListenerSocket();
		~ListenerSocket();

		int		get_fd_listener();

		int			get_domain() const;
		std::string	get_port() const;
		int			get_flag() const;
		int			get_worker_connections() const;
		int			get_type() const;
		void		set_domain(int domain);
		void		set_port(std::string port);
		void		set_flag(int flag);
		void		set_worker_connections(int connections);
		void		set_type(int type);

		void		addrinfo(int domain, int type, int flag, std::string port);
		void		create_fd();
		void		bind_fd_to_port();
		void		listen_fd();
};


#endif