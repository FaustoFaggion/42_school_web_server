#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	protected:
		std::string					_method;
		std::string					_url;
		std::string					_protocol;
		std::string					_content_type;
		std::string					_content_length;
		std::string					_server_name;
		std::string					_server_port;
		std::string					_user_agent;
		std::string					_http_host;
		std::string					_http_accept;
		std::string					_http_accept_encoding;
		std::string					_http_accept_language;
		std::string					_query_string;
		std::string					_path_info;
		std::string					_request_uri;
		std::string					_remote_host;


		std::vector<std::string>	_cgi_envs;

		
	public:
		HttpRequest();
		~HttpRequest();


		void						request_parser(std::string request);
		void						cgi_envs_parser(std::string html);

		std::string					getMethod() const;
		std::string					getUrl() const;
		std::string					getProtocol() const;
		std::string					getContentType() const;
		std::vector<std::string>	getCgiEnvs() const;
		void						add_cgi_envs(std::string request, std::string str, std::string env, size_t size, char c);

};

#endif