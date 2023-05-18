#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "main.hpp"
#include "ListenerSocket.hpp"

class WebServ
{
	private:
		ListenerSocket	listener;
	public:
		WebServ();
		~WebServ();
	
	ListenerSocket	getListener() const;
		
};

#endif