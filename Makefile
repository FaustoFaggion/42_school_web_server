NAME		=	webserv

#COMPILATION
CC			=	c++
FLAGS		=	-Wall -Wextra -Werror  -std=c++98 -g3

PATH_SRC	=	./srcs/
PATH_OBJ	=	./obj/

SRC			=	main.cpp\
				ListenerSocket.cpp\
				WebServ.cpp\
				Parser.cpp\
				FileParser.cpp\
				HttpResponse.cpp\
				HttpRequest.cpp\

VPATH		=	$(PATH_SRC)\
				
OBJ			=	$(SRC:%.cpp=$(PATH_OBJ)%.o)
#OBJ		= $(addprefix $(OBJ_PATH), $(notdir $(SRC:.c=.o)))

HEADER		=	WebServ.hpp\
				ListenerSocket.hpp\
				main.hpp\
				Parser.hpp\
				FileParser.hpp\
				HttpResponse.hpp\
				httpRequest.hpp\

all:	$(NAME)

$(PATH_OBJ)%.o:	%.cpp
	mkdir -p $(PATH_OBJ)
	$(CC) $(FLAGS) -c $< -o $@

$(NAME):	$(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)
	rm -rf $(PATH_OBJ)

fclean:	clean
	rm -rf $(NAME)

re:	fclean all

run:	$(NAME)
	./$(NAME)

valgrind: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)

valgrind_: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)