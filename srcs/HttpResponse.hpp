#ifndef HTTPRESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "main.hpp"
#include "HttpRequest.hpp"

class WebServ;

class HttpResponse 
{
	private:
		std::map<std::string, directive>	locations;
		std::vector<std::string>			_indexes;

	public:

		HttpResponse(std::map<std::string, directive> locations, std::vector<std::string> indexes);
		~HttpResponse();
		
		void			response_parser(std::string &request);
		std::string		looking_for_path(std::string path);
		void			chk_indexies(std::string path, std::string &html);
		void			diretory_list(std::stringstream &buff, std::string path, std::string html);
		void			buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html);
		void			http_response_syntax(std::string status, std::string &request, std::stringstream &buff, std::string content_type);

};

#endif