#ifndef HTTPRESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "main.hpp"
#include "HttpRequest.hpp"

class HttpResponse
{
	private:
		std::map<std::string, directive>	locations;
		std::vector<std::string>			_indexes;
		std::vector<std::string>			_cgi_envs;

	public:

		HttpResponse(std::map<std::string, directive> locations, std::vector<std::string> indexes);
		~HttpResponse();
		
		void			response_parser(t_client &client);
		std::string		looking_for_path(t_client &client);
		void			chk_indexies(t_client &client, std::string &html);
		void			diretory_list(std::stringstream &buff, std::string path, std::string html);
		void			buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html);
		void			http_response_syntax(std::string status, std::string &request, std::stringstream &buff, std::string content_type);

		void			cgi_envs_parser(t_client client, std::string html);
		void			exec_cgi(std::string &html, t_client &client);
};

#endif