#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "main.hpp"

class HttpRequest 
{
	private:
		std::string					_request_method;
		std::string					_url;
		std::string					_server_protocol;
		std::string					_content_type;

		std::vector<std::string>	_cgi_envs;
		
	public:
		HttpRequest();
		~HttpRequest();


		void						request_parser(std::string request);
		void						cgi_envs_parser(std::string request);

		std::string					getMethod() const;
		std::string					getUrl() const;
		std::string					getProtocol() const;
		std::string					getContentType() const;
		std::vector<std::string>	getCgiEnvs() const;
		void						add_cgi_envs(std::string request, std::string str, std::string env, size_t size, char c);

};

#endif