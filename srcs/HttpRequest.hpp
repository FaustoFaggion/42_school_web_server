#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	private:
		std::string		_method;
		std::string		_url;
		std::string		_protocol;
		
	public:
		HttpRequest();
		~HttpRequest();


		void			request_parser(std::string request);
		std::string		getMethod() const;
		std::string		getUrl() const;
		std::string		getProtocol() const;

};

#endif