#include "main.hpp"
#include "WebServ.hpp"
#include "FileParser.hpp"
#include "Monitor.hpp"

int	main(int argc, char *argv[])
{
	Monitor	r;
	
	if (argc != 2)
		std::cout << "Wrong number of arguments" << std::endl;
	else
		r.initialize_monitor(argv[1]);
	return (0);
}
