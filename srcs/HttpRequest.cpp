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
	_boundary = "";
	_content = "";

	_file = "";
	_file_extension = "";
}

HttpRequest::~HttpRequest()
{

}

std::string	HttpRequest::parse_line(std::string &request, std::string start, std::string end)
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
		getline(iss0, _boundary, ' ');
		_content_type += " " + _boundary;
	}

	/*CONTENT_LENGTH*/
	pos = request.find("Content-Length: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Content-Length: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, _content_length, ' ');
	}

	/*CONTENT*/
	pos = _content_type.find("boundary");
	if (pos != _content_type.npos)
	{
		pos = request.find_last_of(_boundary);
		start = pos - atoi(_content_length.c_str());
		end = atoi(_content_length.c_str());
		_content = request.substr(start, end);
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

	/*FILE, FILE_EXTENSION*/
	pos = _url.find(".");
	if (pos != _url.npos)
	{
		end = _url.size() - pos;
		_file_extension = _url.substr(pos, end);
		
		start = _url.find_last_of("/");
		end = _url.size() - start;
		_file = _url.substr(start, end);
	}

		std::cout << "_method: " << _method << "\n";
		std::cout << "_url: " << _url << "\n";
		std::cout << "_protocol: " << _protocol << "\n";
		std::cout << "_content_type: " << _content_type << "\n";
		std::cout << "_content_length: " << _content_length << "\n";
		std::cout << "_server_name: " << _server_name << "\n";
		std::cout << "_server_port: " << _server_port << "\n";
		std::cout << "_user_agent: " << _user_agent << "\n";
		std::cout << "_http_host: " << _http_host << "\n";
		std::cout << "_http_accept: " << _http_accept << "\n";
		std::cout << "_http_accept_encoding: " << _http_accept_encoding << "\n";
		std::cout << "_http_accept_language: " << _http_accept_language << "\n";
		std::cout << "_query_string: " << _query_string << "\n";
		std::cout << "_path_info: " << _path_info << "\n";
		std::cout << "_request_uri: " << _request_uri << "\n";
		std::cout << "_remote_host: " << _remote_host << "\n";
		std::cout << "_boundary: " << _boundary << "\n";
		std::cout << "_content: " << _content << "\n";
		std::cout << "_file: " << _file << "\n";
		std::cout << "_file_extension: " << _file_extension << "\n";

}