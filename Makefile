NAME		=	webserv

#COMPILATION
CC			=	c++
FLAGS		=	-Wall -Wextra -Werror  -std=c++98 -g3

PATH_SRC	=	./srcs/
PATH_OBJ	=	./obj/

SRC			=	main.cpp\
				ListenerSocket.cpp\
				WebServ.cpp\

VPATH		=	$(PATH_SRC)\
				
OBJ			=	$(SRC:%.cpp=$(PATH_OBJ)%.o)
#OBJ		= $(addprefix $(OBJ_PATH), $(notdir $(SRC:.c=.o)))

HEADER		=	WebServ.hpp\
				ListenerSocket.hpp\
				
all:	$(NAME)

$(PATH_OBJ)%.o:	%.cpp $(HEADER)
	mkdir -p $(PATH_OBJ)
	$(CC) $(FLAGS) -c $< -o $@

$(NAME):	$(OBJ) $(HEADER)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)
# -I $(HEADER)

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