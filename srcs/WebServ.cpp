#include "WebServ.hpp"

WebServ::WebServ()
{
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "0";
	_fd_listener = 0;
}

WebServ::WebServ(int domain, int type, int flag, std::string port)
{
	_listener.addrinfo(domain, type, flag, port);
}

WebServ::~WebServ()
{

}

ListenerSocket	WebServ::getListener() const
{
	return(_listener);
}

int				WebServ::getFdListener() const
{
	return (_fd_listener);
}

void	WebServ::setup_server(int domain, int type, int flag, std::string port)
{
	_listener.addrinfo(domain, type, flag, port);
}

void	WebServ::create_listener_socket()
{
	_listener.create_fd();
	_listener.bind_fd_to_port();
	_listener.listen_fd();
	_fd_listener = _listener.get_fd_listener();
}
