#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
	_method = "";
	_url = "";
	_protocol = "";
	_content_type = "";
	_content_length = "";
	_server_name = "";
	_server_port = "";
	_user_agent = "";
	_http_host = "";
	_http_accept = "";
	_http_accept_encoding = "";
	_http_accept_language = "";
	_query_string = "";
	_path_info = "";
	_request_uri = "";
	_remote_host = "";
}

HttpRequest::~HttpRequest()
{

}

std::string		HttpRequest::getMethod() const
{
	return (_method);
}

std::string		HttpRequest::getUrl() const
{
	return (_url);
}

std::string		HttpRequest::getProtocol() const
{
	return(_protocol);
}

std::string		HttpRequest::getContentType() const
{
	return (_content_type);
}

std::vector<std::string>	HttpRequest::getCgiEnvs() const
{
	return (_cgi_envs);
}

std::string	parse_line(std::string &request, std::string start, std::string end)
{
	size_t		requestLineEnd;
	size_t		requestLineStart;
	std::string requestLine;
	
	// Find the start and end of the request line
	requestLineStart = request.find(start);
	if (requestLineStart != request.npos)
	{
		requestLineEnd = request.find(end, requestLineStart);
		requestLine = request.substr(requestLineStart,(requestLineEnd - requestLineStart));
	}
	else
		requestLine = "";

	return (requestLine);
}

void	HttpRequest::add_cgi_envs(std::string request, std::string str, std::string env, size_t size, char c)
{
	std::string	requestLine;
	std::string	value;

	requestLine = parse_line(request, str, "\r\n");
	if (requestLine != "")
	{
		std::istringstream iss(requestLine);
		getline(iss, value, c);
		getline(iss, value, c);
		_cgi_envs.push_back(requestLine.replace(0, size, env));
	}
	else
	{
		value += env + "NULL";
		_cgi_envs.push_back(value);
	}
}

void	HttpRequest::cgi_envs_parser(std::string html)
{
	std::cout << "\nCGI_ENVS_PARSER FUNCTION\n\n";

	std::string		env;
	
	/*SET REDIRECT_STATUS ENV*/
	_cgi_envs.push_back("REDIRECT_STATUS=200");
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	// _cgi_envs.push_back("AUTH_TYPE=NULL");
	// setenv("AUTY_TYPE", "NULL", 1);
	_cgi_envs.push_back("AUTY_TYPE=NULL");
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_LENGTH*/
	env = "CONTENT_LENGTH=" + _content_length;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_TYPE*/
	env = "CONTENT_TYPE=" + _content_type;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*GATWAY_INTERFACE*/ //?????????????????????????????????
	_cgi_envs.push_back("GATWAY_INTERFACE=CGI/7.4");

	/*HTTP_ACCEPT*/
	env = "HTTP_ACCEPT=" + _http_accept;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_ENCODING*/
	env = "HTTP_ACCEPT_ENCODING=" + _http_accept_encoding;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_LANGUAGE*/
	env = "HTTP_ACCEPT_LANGUAGE=" + _http_accept_language;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_HOST*/
	env = "HTTP_HOST=" + _http_host;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_NAME*/
	env = "SERVER_NAME=" + _server_name;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_PORT*/
	env = "SERVER_PORT=" + _server_port;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";


	/*HTTP_USER_AGENT*/
	env = "HTTP_USER_AGENT=" + _user_agent;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*QUERY_STRING*/
	env = "QUERY_STRING=" + _query_string;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*PATH_INFO*/
	env = "PATH_INFO=" + _path_info;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REQUEST_URI*/
	env = "REQUEST_URI=" + _request_uri;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*REQUEST_METHOD*/
	env = "REQUEST_METHOD=" + _method;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SERVER_PROTOCOL*/
	env = "SERVER_PROTOCOL=" + _protocol;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REMOTE_HOST*/
	env = "REMOTE_HOST=" + _remote_host;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SCRIPT_NAME*/
	env = "SCRIPT_NAME=/usr/bin/php-cgi";
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SCRIPT_FILENAME*/
	env = "SCRIPT_FILENAME=" + html;
	_cgi_envs.push_back(env);

	env = "DOCUMENT_ROOT=/home/fausto/42SP/webserv_git";
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	_cgi_envs.push_back("REDIRECT_STATUS=true");
	std::cout << *(_cgi_envs.end() - 1) << "\n";
}

void		HttpRequest::request_parser(std::string request)
{

	std::cout << "\nREQUEST_PARSER FUNCTION\n";

	size_t			pos, start, end;
	std::string		tmp;
	std::string		requestLine;

	/*METHOD, URL, PROTOCOL*/
	size_t requestLineEnd = request.find("\r\n");
	requestLine = request.substr(0, requestLineEnd);
	std::istringstream iss0(requestLine);
	iss0 >> _method >> _url >> _protocol;

	/*URL, QUERY_STRING*/
	pos = _url.find("?");
	if (pos != _url.npos)
	{
		std::istringstream iss0(_url);
		getline(iss0, tmp, '?');
		getline(iss0, _query_string, '?');
		_url = tmp;
	}
	
	/*PATH_INFO, REQUEST_URI*/
	{
		start = _url.find("/");
		end = _url.size() - start;
		_path_info = _url.substr(start, end);
		_request_uri = _url.substr(start, end);
	}

	/*CONTENT_TYPE*/
	pos = request.find("Content-Type: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Content-Type: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _content_type, ' ');
		getline(iss0, tmp, ' ');
		_content_type += " " + tmp;
	}

	/*CONTENT_TYPE*/
	pos = request.find("Content-Length: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Content-Length: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _content_length, ' ');
	}

	/*HTTP_HOST*/
	pos = request.find("Host: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Host: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _http_host, ' ');
		std::istringstream iss1(_http_host);
		getline(iss1, _server_name, ':');
		getline(iss1, _server_port, ':');
	}

	/*USER_AGENT*/
	pos = request.find("User-Agent: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "User-Agent: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _user_agent, ' ');
	}

	/*ACCEPT*/
	pos = request.find("Accept: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Accept: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _http_accept, ' ');
	}

	/*ACCEPT_LANGUAGE*/
	pos = request.find("Accept-Language: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Accept-Language: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _http_accept_language, ' ');
	}

	/*ACCEPT_ENCODING*/
	pos = request.find("Accept-Encoding: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Accept-Encoding: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _http_accept_encoding, ' ');
	}

	/*REMOTE_HOST*/
	pos = request.find("Origin: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Origin: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _remote_host, ' ');
	}

}