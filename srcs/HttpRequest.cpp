#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{

}

HttpRequest::~HttpRequest()
{

}

std::string		HttpRequest::getMethod() const
{
	return (_request_method);
}

std::string		HttpRequest::getUrl() const
{
	return (_url);
}

std::string		HttpRequest::getProtocol() const
{
	return(_server_protocol);
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

void	HttpRequest::cgi_envs_parser(std::string request)
{
	std::string		requestLine;
	std::string		tmp;

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	_cgi_envs.push_back("AUTH_TYPE=NULL");

	
	/*CONTENT_LENGTH*/
	requestLine = parse_line(request, "Content-Length: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "CONTENT_LENGTH=");
		_cgi_envs.push_back(requestLine);
	}

	/*CONTENT_TYPE*/
	requestLine = parse_line(request, "Content-Type: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "CONTENT_TYPE=");
		_cgi_envs.push_back(requestLine);
	}

	/*GATWAY_INTERFACE*/ //?????????????????????????????????
	_cgi_envs.push_back("GATWAY_INTERFACE=CGI/1.1");

	/*HTTP_ACCEPT*/
	requestLine = parse_line(request, "Accept: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "HTTP_ACCEPT=");
		_cgi_envs.push_back(requestLine);
	}

	/*HTTP_ACCEPT_ENCODING*/
	requestLine = parse_line(request, "Accept-Encoding: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "HTTP_ACCEPT_ENCODING=");
		_cgi_envs.push_back(requestLine);
	}

	/*HTTP_ACCEPT_LANGUAGE*/
	requestLine = parse_line(request, "Accept-Language: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "HTTP_ACCEPT_LANGUAGE=");
		_cgi_envs.push_back(requestLine);
	}

	/*HTTP_HOST*/
	requestLine = parse_line(request, "Host: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "HTTP_HOST=");
		_cgi_envs.push_back(requestLine);
	}

	/*HTTP_USER_AGENT*/
	requestLine = parse_line(request, "User-Agent: ", "\r\n");
	if (requestLine != "")
	{
		size_t pos = requestLine.find(": ");
		requestLine.replace(0, pos + 2, "HTTP_USER_AGENT=");
		_cgi_envs.push_back(requestLine);
	}

	/*PATH_INFO*/
	size_t	end = request.find("\r\n");
	requestLine = request.substr(0, end);
	{
		std::string	method, url, protocol, tmp0, tmp1;
		std::istringstream iss0(requestLine);

		iss0 >> method >> url >> protocol;
		size_t pos = _url.find("?");
		if (pos != _url.npos)
		{
			
			std::istringstream iss0(_url);
			getline(iss0, _url, '?');
			getline(iss0, tmp0, '?');
			getline(iss0, tmp1, '=');
			tmp0 = "PATH_INFO=" + tmp1;
			_cgi_envs.push_back(tmp0);
			getline(iss0, tmp1, '=');
			tmp0 = "QUERY_STRING=" + tmp1;
			_cgi_envs.push_back(tmp0);

		}
		tmp0 = "REQUEST_METHOD=" + method;
		_cgi_envs.push_back(tmp0);
		tmp0 = "SERVER_PROTOCOL=" + protocol;
	}

	std::cout << "\nCGI VAriables\n\n";
	for (std::vector<std::string>::iterator it = _cgi_envs.begin(); it != _cgi_envs.end(); it++)
		std::cout << *it << "\n";
}

void		HttpRequest::request_parser(std::string request)
{

	std::cout << "REQUEST_PARSER FUNCTION\n";

	std::string		tmp;
	std::string		requestLine;

	/*METHOD, URL, PROTOCOL*/
	size_t requestLineEnd = request.find("\r\n");
	requestLine = request.substr(0, requestLineEnd);
	std::istringstream iss0(requestLine);
	iss0 >> _request_method >> _url >> _server_protocol;

	size_t pos = _url.find("?");
	if (pos != _url.npos)
	{
		std::istringstream iss0(_url);
		getline(iss0, _url, '?');
	}

	cgi_envs_parser(request);
}