#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
	// _method = "";
	// _url = "";
	// _protocol = "";
	// _content_type = "";
	// _content_length = "";
	// _server_name = "";
	// _server_port = "";
	// _user_agent = "";
	// _http_host = "";
	// _http_accept = "";
	// _http_accept_encoding = "";
	// _http_accept_language = "";
	// _query_string = "";
	// _path_info = "";
	// _request_uri = "";
	// _remote_host = "";
	// _boundary = "";
	// _content = "";

	// _url_file = "";
	// _url_file_extension = "";
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

void		HttpRequest::request_parser(t_client &client)
{

	std::cout << "\nREQUEST_PARSER FUNCTION\n";

	size_t			pos, start, end;
	std::string		tmp;
	std::string		requestLine;

	/*METHOD, URL, PROTOCOL*/
	size_t requestLineEnd = client._header.find("\r\n");
	requestLine = client._header.substr(0, requestLineEnd);
	std::istringstream iss0(requestLine);
	iss0 >> client._method >> client._url >> client._protocol;

	/*CHECK METHOD ERROR*/
	if (client._method.compare("POST") != 0 \
		&& client._method.compare("GET") != 0 && client._method.compare("DELETE") != 0)
	{
		client._status_code = "501";
		client._status_msg = "Not Implemented";
	}
	/*URL, QUERY_STRING*/
	pos = client._url.find("?");
	if (pos != client._url.npos)
	{
		std::istringstream iss0(client._url);
		getline(iss0, tmp, '?');
		getline(iss0, client._query_string, '?');
		client._url = tmp;
	}
	
	/*PATH_INFO, REQUEST_URI*/
	{
		start = client._url.find("/");
		end = client._url.size() - start;
		client._path_info = client._url.substr(start, end);
		client._request_uri = client._url.substr(start, end);
	}

	/*CONTENT_TYPE*/
	pos = client._header.find("Content-Type: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Content-Type: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._content_type, ' ');
		getline(iss0, client._boundary, ' ');
		client._content_type += " " + client._boundary;
	}

	/*CONTENT_LENGTH*/
	pos = client._header.find("Content-Length: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Content-Length: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._content_length, ' ');
	}
	/*CHECK ERROR CONTENT_LENGTH*/
	if (client._status_code.compare("200") == 0 && atoi(client._content_length.c_str()) > atoi(client._max_body_length.c_str()))
	{
		client._status_code = "400";
		client._status_msg = "Bad Request";
	}
	/*HTTP_HOST*/
	pos = client._header.find("Host: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Host: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_host, ' ');
		std::istringstream iss1(client._http_host);
		getline(iss1, client._server_name, ':');
		getline(iss1, client._server_port, ':');
	}

	/*USER_AGENT*/
	pos = client._header.find("User-Agent: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "User-Agent: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._user_agent, ' ');
	}

	/*ACCEPT*/
	pos = client._header.find("Accept: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept, ' ');
	}

	/*ACCEPT_LANGUAGE*/
	pos = client._header.find("Accept-Language: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept-Language: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept_language, ' ');
	}

	/*ACCEPT_ENCODING*/
	pos = client._header.find("Accept-Encoding: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept-Encoding: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept_encoding, ' ');
	}

	/*REMOTE_HOST*/
	pos = client._header.find("Origin: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Origin: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._remote_host, ' ');
	}

	/*_URL_FILE, _URL_FILE_EXTENSION, _URL_LOCATION*/
	pos = client._url.find(".");
	if (pos != client._url.npos)
	{
		end = client._url.size() - pos;
		client._url_file_extension = client._url.substr(pos, end);
		
		start = client._url.find_last_of("/");
		end = client._url.size() - start;
		client._url_file = client._url.substr(start, end);

		end = start;
		client._url_location = client._url.substr(0, end);
		if (client._url_location == "")
			client._url_location = '/';
	}

		// std::cout << "_method: " << client._method << "\n";
		// std::cout << "_url: " << client._url << "\n";
		// std::cout << "_protocol: " << client._protocol << "\n";
		// std::cout << "_content_type: " << client._content_type << "\n";
		std::cout << "_content_length: " << client._content_length << "\n";
		std::cout << "_max_body_length: " << client._max_body_length << "\n";
		// std::cout << "_server_name: " << client._server_name << "\n";
		// std::cout << "_server_port: " << client._server_port << "\n";
		// std::cout << "_user_agent: " << client._user_agent << "\n";
		// std::cout << "_http_host: " << client._http_host << "\n";
		// std::cout << "_http_accept: " << client._http_accept << "\n";
		// std::cout << "_http_accept_encoding: " << client._http_accept_encoding << "\n";
		// std::cout << "_http_accept_language: " << client._http_accept_language << "\n";
		// std::cout << "_query_string: " << client._query_string << "\n";
		// std::cout << "_path_info: " << client._path_info << "\n";
		// std::cout << "_request_uri: " << client._request_uri << "\n";
		// std::cout << "_remote_host: " << client._remote_host << "\n";
		// std::cout << "_boundary: " << client._boundary << "\n";
		// std::cout << "_content: " << client._content << "\n";
		// std::cout << "_url_file: " << client._url_file << "\n";
		// std::cout << "_url_file_extension: " << client._url_file_extension << "\n";
		// std::cout << "_url_location: " << client._url_location << "\n";
		std::cout << "_status_code: " << client._status_code << "\n";
}

void			HttpRequest::split_header_and_body(t_client &client, std::string buff)
{
	size_t	pos;

	pos = buff.find("\r\n\r\n");
	pos += 4;
	client._header += buff.substr(0, pos);
	client._body = buff.substr(pos, (buff.size() - pos));
}