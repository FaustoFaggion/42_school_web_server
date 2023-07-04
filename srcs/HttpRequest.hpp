#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	protected:
		// std::string					_method;
		// std::string					_url;
		// std::string					_protocol;
		// std::string					_content_type;
		// std::string					_content_length;
		// std::string					_server_name;
		// std::string					_server_port;
		// std::string					_user_agent;
		// std::string					_http_host;
		// std::string					_http_accept;
		// std::string					_http_accept_encoding;
		// std::string					_http_accept_language;
		// std::string					_query_string;
		// std::string					_path_info;
		// std::string					_request_uri;
		// std::string					_remote_host;
		// std::string					_boundary;
		// std::string					_content;
		// std::string					_url_file; /*FILE IN THE END OF URL REQUEST*/
		// std::string					_url_file_extension; /*IF .PHP RUN CGI*/
		// std::string					_url_location;

	public:
		HttpRequest();
		~HttpRequest();

		void						request_parser(t_client &client);
		std::string					parse_line(std::string &request, std::string start, std::string end);
};

#endif