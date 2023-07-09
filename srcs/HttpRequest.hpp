#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	public:
		HttpRequest();
		~HttpRequest();

		void			request_parser(t_client &client);
		std::string		parse_line(std::string &request, std::string start, std::string end);
		void			split_header_and_content(t_client &client, std::string buff);
};

#endif