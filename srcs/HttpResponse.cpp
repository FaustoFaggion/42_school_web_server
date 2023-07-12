#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{

}


HttpResponse::~HttpResponse()
{

}

void	HttpResponse::chk_indexies(t_client &client, std::string &html, std::map<std::string, directive> &locations, std::vector<std::string> indexes)
{
	std::cout << "\nCHK_INDEXIES FUNCTION\n";

	bool	flag = false;
	size_t	i = 0;
		
	while (flag == false && i < locations[client._url]._index_block.size())
	{
		html = locations[client._url]._server_path + "/" + locations[client._url]._index_block.at(i);
		if(access(html.c_str(), F_OK) == 0)
		{
			flag = true;
			locations[client._url]._path_ok = true;
			std::cout << "1 -path_ok = " << locations[client._url]._path_ok << "\n";
			std::cout << "index found block directive: " << html << "\n";
		}
		i++;
	}
	i = 0;
	if (flag == false)
	{
		while (flag == false && i < indexes.size())
		{
			html = locations[client._url]._server_path + "/" + indexes.at(i);
			if(access(html.c_str(), F_OK) == 0)
			{
				flag = true;
				locations[client._url]._path_ok = true;
				std::cout << "2 -path_ok = " << locations[client._url]._path_ok << "\n";
				std::cout << "index found simple directive: " << html << "\n";
			}
			i++;
		}
	}
	if (flag == false)
	{
		locations[client._url]._path_ok = false;
		client._status_code = "404";
		client._status_msg = "Page Not Found";
		html = locations[client._url]._server_path;
		std::cout << "3 -path_ok = " << locations[client._url]._path_ok << "\n";
		std::cout << "index not found: " << html << "\n";
	}

}

void	HttpResponse::looking_for_path(t_client &client, std::map<std::string, directive> &locations, std::vector<std::string> indexes)
{
	std::cout << "\nLOOKING_FOR_PATH FUNCTION\n";

	/*IF REQUEST IS A LOCATION, APPEND INDEX.HTML FILES DEFINED INTO CONFIGURATION FILE*/
	if(locations.find(client._url) != locations.end())
	{
		std::cout << "path on location map found: " << client._url << "\n";
		chk_indexies(client, client._server_path, locations, indexes);
		if (locations[client._url]._path_ok == true)
			return ;
	}
	
	/*CHECK FOR FILE IN THE END OF THE PATH REQUEST*/
	if (client._url_file == "")
	{
		client._server_path = locations[client._url]._server_path;
		client._status_code = "404";
		client._status_msg = "Page Not Found";
		locations[client._url]._path_ok = false;
		std::cout << "path_ok = " << locations[client._url]._path_ok << "\n";
		std::cout << "path on location map not found: " << client._server_path << "\n";
		return ;
	}

	if(locations.find(client._url_location) != locations.end())
	{
		if (client._url_file_extension == ".php")
			client._server_path = "php-cgi" + client._url_file;
		else
			client._server_path = locations[client._url_location]._server_path + client._url_file;
		std::cout << "html: " << client._server_path << "\n";
		client._url = client._url_location;
		if (access(client._server_path.c_str(), F_OK) == 0)
		{
			locations[client._url_location]._path_ok = true;
			std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
			std::cout << "find path on location map and file: " << client._url_location <<"\n";
		}
		else
		{
			client._status_code = "404";
			client._status_msg = "Page Not Found";
			locations[client._url_location]._path_ok = false;
			std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
			std::cout << "find path on location map but not file: " << client._url_location <<"\n";
		}
	}
	else
	{
		// html = locations[request_path]._server_path;
		client._status_code = "404";
		client._status_msg = "Page Not Found";
		locations[client._url_location]._path_ok = false;
		std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
		std::cout << "path not found on location map after extract file: " << client._url_location << "\n";
	}

	/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = client._server_path.find("//");
		if (pos != client._server_path.npos)
			client._server_path.replace(pos, 1, "");
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

void	HttpResponse::http_response_error(t_client &client)
{
	client._response = "HTTP/1.1" + client._status_code + client._status_msg;
	client._response += client._content_type;
	client._response += "Connection: close\r\n";
	client._response += "\r\n";

	client._response += "<!DOCTYPE html>\n \
	<html lang='en'>\n \
		<head>\n \
			<meta charset='UTF-8'> \n \
			<meta http-equiv='X-UA-Compatible' content='IE=edge'> \n \
			<meta name='viewport' content='width=device-width, initial-scale=1.0'> \n \
			<title>Error</title> \n \
			<link rel='icon' href='favicon.ico' type='image/x-icon'> \n \
		<style> \n \
			body { \n \
			font-family: Arial, sans-serif; \n \
			margin: 0; \n \
			padding: 0; \n \
			background-color: #f4f4f4; \n \
		} \n \
		\n \
		.container { \n \
			display: flex; \n \
			align-items: center; \n \
			justify-content: center; \n \
			height: 100vh; \n \
			text-align: center; \n \
		} \n \
		\n \
		.error-code { \n \
			font-size: 100px; \n \
			font-weight: bold; \n \
			margin-bottom: 20px; \n \
		} \n \
		\n \
		.error-message { \n \
			font-size: 30px; \n \
			color: #888; \n \
		} \n \
		</style> \n \
	</head> \n \
	<body> \n \
		<div class='container'> \n \
			<div> \n \
			<h1 class='error-code'>Error " + client._status_code + "</h1> \n \
			<p class='error-message'>" + client._status_msg + "</p> \n \
			</div> \n \
		</div> \n \
	</body> \n \
</html> \n";
}

void	HttpResponse::cgi_envs_parser(t_client client, std::string html)
{
	std::cout << "\nCGI_ENVS_PARSER FUNCTION\n\n";

	std::string		env;
	
	/*SET REDIRECT_STATUS ENV*/
	_cgi_envs.push_back("REDIRECT_STATUS=200");
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	// _cgi_envs.push_back("AUTH_TYPE=NULL");
	// setenv("AUTY_TYPE", "NULL", 1);
	_cgi_envs.push_back("AUTY_TYPE=NULL");
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_LENGTH*/
	env = "CONTENT_LENGTH=" + client._content_length;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_TYPE*/
	env = "CONTENT_TYPE=" + client._content_type;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*GATWAY_INTERFACE*/ //?????????????????????????????????
	_cgi_envs.push_back("GATEWAY_INTERFACE=CGI/7.4");
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT*/
	env = "HTTP_ACCEPT=" + client._http_accept;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_ENCODING*/
	env = "HTTP_ACCEPT_ENCODING=" + client._http_accept_encoding;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_LANGUAGE*/
	env = "HTTP_ACCEPT_LANGUAGE=" + client._http_accept_language;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_HOST*/
	env = "HTTP_HOST=" + client._http_host;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_NAME*/
	env = "SERVER_NAME=" + client._server_name;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_PORT*/
	env = "SERVER_PORT=" + client._server_port;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";


	/*HTTP_USER_AGENT*/
	env = "HTTP_USER_AGENT=" + client._user_agent;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*QUERY_STRING*/
	env = "QUERY_STRING=" + client._query_string;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*PATH_INFO*/
	env = "PATH_INFO=" + client._path_info;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REQUEST_URI*/
	env = "REQUEST_URI=" + client._request_uri;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*REQUEST_METHOD*/
	env = "REQUEST_METHOD=" + client._method;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SERVER_PROTOCOL*/
	env = "SERVER_PROTOCOL=" + client._protocol;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REMOTE_HOST*/
	env = "REMOTE_HOST=" + client._remote_host;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SCRIPT_NAME*/
	env = "SCRIPT_NAME=/usr/bin/php-cgi";
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SCRIPT_FILENAME*/
	env = "SCRIPT_FILENAME=" + html;
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	env = "DOCUMENT_ROOT=/home/fausto/42SP/webserv_git";
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	_cgi_envs.push_back("REDIRECT_STATUS=true");
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	std::ostringstream oss;
    oss << client._upload_buff_size;
	env = "BUFFER_SIZE=" + oss.str();
	_cgi_envs.push_back(env);
	// std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*ADD ENVP TO CGI_ENVS */
	extern char** environ; 
	for (int i = 0; environ[i] != NULL; i++)
		_cgi_envs.push_back(environ[i]);

	/*PRINT CONTENT TO DEBUG*/
	// std::cout << "CONTENT\n\n" << client._content << "\n\n";
}

// void	HttpResponse::exec_cgi(std::string &html, t_client &client)
// {
// 	std::cout << "\nEXEC_CGI FUNCTION" << html << "\n\n";

// 	int				pid;
// 	char			*arg2[3];
	
// 	arg2[0] = (char *)"/usr/bin/php-cgi7.4";
// 	arg2[1] = (char *)html.c_str();
// 	arg2[2] = NULL;

// 	if (pipe(client.pipe0) == -1)
// 		exit(write(1, "pipe error\n", 11));
// 	if (pipe(client.pipe1) == -1)
// 		exit(write(1, "pipe error\n", 11));
	
// 	pid = fork();
// 	if (pid < 0)
// 		exit(write(1, "fork error\n", 11));
// 	if (pid == 0)
// 	{

// 		cgi_envs_parser(client, html);

// 		/*CREATE ENVP_CGI ARRAY TO EXECVE*/
// 		char *envp_cgi[_cgi_envs.size() + 1];
// 		size_t i = 0;
// 		while (i < _cgi_envs.size()) {
// 			envp_cgi[i] = (char *)_cgi_envs.at(i).c_str();
// 			i++;
// 		}
// 		envp_cgi[i] = NULL;


// 		if (client._method == "POST")
// 			dup2(client.pipe0[0], STDIN_FILENO);

// 		dup2(client.pipe1[1], STDOUT_FILENO);
// 		close(client.pipe0[0]);
// 		close(client.pipe0[1]);
// 		close(client.pipe1[0]);
// 		close(client.pipe1[1]);
// 		if (execve(arg2[0], arg2, envp_cgi) == -1)
// 		{
// 			write(2, strerror(errno), strlen(strerror(errno)));
// 			exit(1);
// 		}
// 	}

// 	if (client._method == "POST")
// 	{
// 		char	buff[client._upload_buff_size];

// 		std::cout << "CONTENT: " << client._content << "\n\n";
// 		std::cout << "CONTENT_SIZE: " << client._content.size() << "\n\n";
// 		std::cout << "UPLOAD_BUFF_SIZE: " << client._upload_buff_size << "\n\n";
// 		close(client.pipe0[0]);
// 		write(client.pipe0[1], client._content.c_str(), client._content.size());
// 		client._upload_content_size = client._content.size();
// 		std::cout << client._content.size() << " : " << client._upload_buff_size << "\n\n";
// 		while (client._upload_content_size < (size_t)atoi(client._content_length.c_str()))
// 		{
// 			memset (&buff, '\0', sizeof (buff));
// 			/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
// 			ssize_t numbytes = recv (client.fd, &buff, sizeof(buff), 0);
// 			// std::cout << "BUFF: " << buff << "\n\n";
// 			write(client.pipe0[1], buff, numbytes);
// 			client._upload_content_size += numbytes;
// 			std::cout << client._upload_content_size << " : " << numbytes << " > ";
// 		}
// 		close(client.pipe0[1]);
// 	}
// 	waitpid(pid, NULL, 0);
// 	client._response = "HTTP/1.1 200 OK\r\n";
// 	std::cout << "enter while\n";
// 	close(client.pipe1[1]);
// 	std::stringstream phpOutput;
// 	char		buffer[client._upload_buff_size];
// 	ssize_t bytesRead;
// 	while ((bytesRead = read(client.pipe1[0], buffer, sizeof(buffer))) != 0)
// 	{
// 		phpOutput.write(buffer, bytesRead);
// 	}
// 	client._response += phpOutput.str();
// 	std::cout << "out while\n";
// 	// std::cout << "request: " << request << "\n";
// 	close(client.pipe1[0]);

// }

void	HttpResponse::response_parser(t_client &client, std::map<std::string, directive> locations)
{
	std::cout << "\nRESPONSE_PARSE FUNCTION\n";

	// _indexes = indexes;

	std::fstream			conf_file;
	std::stringstream		buff;

	std::cout << "\nRESPONSE_PARSE FUNCTION\n";

	std::cout << "\n";
	for (std::map<std::string, directive>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		std::cout << (*it).first << " : " << (*it).second._server_path << "\n";
	}
	std::cout << "\n";
	std::cout << "Method: " << client._method << std::endl;
	std::cout << "Path: " << client._url << std::endl;
	std::cout << "Protocol: " << client._protocol << std::endl;
	std::cout << "client._server_path: " << client._server_path << std::endl;
	std::cout << "\n";

	if (client._status_code.compare("200") == 0 || locations[client._url]._autoindex == true)
	{
		if (client._method.compare("GET") == 0)
		{
			if (locations[client._url]._autoindex == true)
			{
				std::cout << "autoindex on\n";
				if (locations[client._url]._path_ok == false)
					diretory_list(buff, client._url, client._server_path);
				else
					buff_file(conf_file, buff, client._server_path);
			}
			else
			{
				std::cout << "autoindex off\n";
				buff_file(conf_file, buff, client._server_path);
			}
			http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
			conf_file.close();
		}
		else if (client._method.compare("POST") == 0)
		{
			buff_file(conf_file, buff, client._server_path);
			http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
			conf_file.close();
		}
		else if (client._method.compare("DELETE") == 0)
		{
			buff_file(conf_file, buff, client._server_path);
			http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
			conf_file.close();
		}
	}
	else
	{
		// buff_file(conf_file, buff, "./locations/test/error.html");
		// http_response_syntax("HTTP/1.1 404 Not Found\r\n", client._response, buff, client._content_type);
		http_response_error (client);
		// conf_file.close();
	}
}