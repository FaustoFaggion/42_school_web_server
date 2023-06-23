#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	private:
		std::string		_request_method;
		std::string		_url;
		std::string		_server_protocol;
		std::string		_server_name;
		std::string		_path_info;
		std::string		_query_string;
		std::string		_server_port;
		
	public:
		HttpRequest();
		~HttpRequest();


		void			request_parser(std::string request);
		std::string		getMethod() const;
		std::string		getUrl() const;
		std::string		getProtocol() const;
		std::string		getServer_name() const;

};

#endif