#include "HttpResponse.hpp"

HttpResponse::HttpResponse(std::map<std::string, directive> locations, std::vector<std::string> indexes)
{
	this->locations = locations;
	_indexes = indexes;
}

HttpResponse::~HttpResponse()
{

}

// void	clean(std::string& str) {
    // Remove leading spaces
//     str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

//     // Remove trailing spaces
//     str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
// 	str += '\0';
// }

void	HttpResponse::chk_indexies(std::string path, std::string &html)
{
	std::cout << "\nCHK_INDEXIES FUNCTION\n";

	bool	flag = false;
	size_t	i = 0;
		
	while (flag == false && i < locations[path]._index_block.size())
	{
		html = locations[path]._server_path + "/" + locations[path]._index_block.at(i);
		if(access(html.c_str(), F_OK) == 0)
		{
			flag = true;
			locations[path]._path_ok = true;
			std::cout << "1 -path_ok = " << locations[path]._path_ok << "\n";
			std::cout << "index found block directive: " << html << "\n";
		}
		i++;
	}
	i = 0;
	if (flag == false)
	{
		while (flag == false && i < _indexes.size())
		{
			html = locations[path]._server_path + "/" + _indexes.at(i);
			if(access(html.c_str(), F_OK) == 0)
			{
				flag = true;
				locations[path]._path_ok = true;
				std::cout << "2 -path_ok = " << locations[path]._path_ok << "\n";
				std::cout << "index found simple directive: " << html << "\n";
			}
			i++;
		}
	}
	if (flag == false)
	{
		locations[path]._path_ok = false;
		html = locations[path]._server_path;
		std::cout << "3 -path_ok = " << locations[path]._path_ok << "\n";
		std::cout << "index not found: " << html << "\n";
	}

}

std::string	HttpResponse::looking_for_path(std::string path)
{
	std::cout << "\nLOOKING_FOR_PATH FUNCTION\n";
	
	std::string	html = "";

	/*IF REQUEST IS A LOCATION, APPEND INDEX.HTML FILES DEFINED INTO CONFIGURATION FILE*/
	if(locations.find(path) != locations.end())
	{
		std::cout << "path on location map found: " << path << "\n";
		chk_indexies(path, html);
		if (locations[path]._path_ok == true)
			return(html);
	}

	/*IF REQUEST PATH NOT MATCH, IT TAKES THE LONGEST PATH THAT INICIATES WITH THE REQUEST*/
	// size_t		s = 0;
	// std::string	comp;
	// if (*(path.end() - 1) != '/')
	// 	comp = path + "/";
	// else
	// 	comp = path;

	// for(std::map<std::string, directive>::iterator it = locations.begin(); it != locations.end(); it++)
	// {
	// 	if ((*it).first.compare(0, comp.size(), comp) == 0)
	// 	{
	// 		if ((*it).first.size() > s)
	// 		{

	// 			html = locations[(*it).first]._server_path + "/" + _indexes.at(0);
	// 			size_t i = 1;
	// 			while (i < _indexes.size() && access(html.c_str(), F_OK) != 0)
	// 			{	
	// 				std::cout << "f a path: " << html << "\n";
	// 				html = locations[(*it).first]._server_path + "/" + _indexes.at(i);
	// 				i++;
	// 			}
	// 			s = (*it).first.size();
	// 		}
	// 	}
	// }

	/*IF PATH MATCH, RETURN HTML STRING*/
	// if (s > 0)
	// {
	// 	std::cout << "find alternative path\n" << html << "\n";
	// 	return (html);
	// }
	
	/*CHECK FOR FILE IN THE END OF THE PATH REQUEST*/
	if (path.find(".", 0) == path.npos)
	{
		html = locations[path]._server_path;
		locations[path]._path_ok = false;
		std::cout << "path_ok = " << locations[path]._path_ok << "\n";
		std::cout << "path on location map not found: " << html << "\n";
		return (html);
	}
	
	/*REMOVE FILE FROM PATH TO LOOKING FOR INTO THE LOCATIONS MAP*/
	size_t start = path.find_last_of("/", path.size());
	std::cout << "start: " << start << std::endl;
	size_t end = path.size();
	std::cout << "end: " << end << std::endl;
	std::string	request_path = path.substr(0, start);
	std::cout << "request_path: " << request_path << std::endl;
	std::string file = path.substr(start, (end - (start)));
	std::cout << "file: " << file << std::endl;

	if(locations.find(request_path) != locations.end())
	{
		html = locations[request_path]._server_path + "/" + file;
		if (access(html.c_str(), F_OK) == 0)
		{
			locations[request_path]._path_ok = true;
			std::cout << "path_ok = " << locations[request_path]._path_ok << "\n";
			std::cout << "find path on location map and file: " << request_path <<"\n";
		}
		else
		{
			locations[request_path]._path_ok = false;
			std::cout << "path_ok = " << locations[request_path]._path_ok << "\n";
			std::cout << "find path on location map but not file: " << request_path <<"\n";
		}
	}
	else
	{
		html = locations[request_path]._server_path;
		locations[request_path]._path_ok = false;
		std::cout << "path_ok = " << locations[request_path]._path_ok << "\n";
		std::cout << "path not found on location map after extract file: " << request_path << "\n";
	}

	/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = html.find("//");
		if (pos != html.npos)
			html.replace(pos, 1, "");

	return (html);
}

void	HttpResponse::diretory_list(std::stringstream &buff, std::string path, std::string html)
{
	std::cout << "DIRETORY_LIST FUNCTION: " << "\n";
	
	buff << "<html>\n";
	buff << "<body>\n";
	buff << "<h1>Directory Listing</h1>\n";
	buff << "<ul>\n";
	DIR* dir;
	struct dirent* entry;
	std::cout << "html: "<< html << "\n";
	dir = opendir(html.c_str());
	if (dir != NULL)
	{
    	while ((entry = readdir(dir)) != NULL)
		{
    		std::string filename = entry->d_name;
    		 std::string link = path + "/" + filename;
    		buff << "<li><a href=\"" << link << "\">" << filename << "</a></li>\n";
    	}
    	closedir(dir);
	}
}

void	HttpResponse::buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html)
{
	conf_file.open(html.c_str() , std::fstream::in);
	if (conf_file.fail())
	{
		conf_file.open("./locations/test/error.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();
	}
	else
		buff << conf_file.rdbuf();
}

void	HttpResponse::http_response_syntax(std::string status, std::string &request, std::stringstream &buff, std::string content_type)
{
	request = status;
	request += content_type;
	request += "Connection: close\r\n";
	request += "\r\n";
	request += buff.str();
	request += "\r\n";

	std::cout << "\nRESPONSE SYNTAX\n\n";
	std::cout << request;
}

void	HttpResponse::response_parser(std::string &request)
{
	std::cout << "\nRESPONSE_PARSE FUNCTION\n";

	std::string method, path, protocol, html, content_type;
	std::fstream			conf_file;
	std::stringstream		buff;

	HttpRequest	rqst;
	rqst.request_parser(request);
	method = rqst.getMethod();
	path = rqst.getUrl();
	protocol = rqst.getProtocol();
	content_type = rqst.getContentType();


	// request_parser(request, method, path, protocol);

	html = looking_for_path(path);

	std::cout << "\n";
	for (std::map<std::string, directive>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		std::cout << (*it).first << " : " << (*it).second._server_path << "\n";
	}
	std::cout << "\n";
	std::cout << "Method: " << method << std::endl;
	std::cout << "Path: " << path << std::endl;
	std::cout << "Protocol: " << protocol << std::endl;
	std::cout << "html: " << html << std::endl;
	std::cout << "\n";

	if (method.compare("GET") == 0)
	{
		if (locations[path]._autoindex == true)
		{
			std::cout << "autoindex on\n";
			if (locations[path]._path_ok == false)
				diretory_list(buff, path, html);
			else
				buff_file(conf_file, buff, html);
		}
		else
		{
			std::cout << "autoindex off\n";
			buff_file(conf_file, buff, html);
		}

		http_response_syntax("HTTP/1.1 200 OK\r\n", request, buff, content_type);
		conf_file.close();
	}
	else if (method.compare("POST") == 0)
	{
		buff_file(conf_file, buff, html);
		http_response_syntax("HTTP/1.1 200 OK\r\n", request, buff, content_type);
		conf_file.close();
	}
	else if (method.compare("DELETE") == 0)
	{
		buff_file(conf_file, buff, html);
		http_response_syntax("HTTP/1.1 200 OK\r\n", request, buff, content_type);
		conf_file.close();
	}
	else
	{
		buff_file(conf_file, buff, "./locations/test/error.html");
		http_response_syntax("HTTP/1.1 404 Not Found\r\n", request, buff, content_type);
		conf_file.close();
	}
}