#SRC += main.cpp
SRC += cgi.cpp
SRC += cgi_handler.cpp
#SRC += config_checker.cpp
#SRC += server.cpp
#SRC += header_handler.cpp

OBJ = $(SRC:.cpp=.o)

CC = clang++

RM = rm -f

NAME = webserv

CGI_OUT = out.html

ifeq ($(df), 0)
CPP_FLAGS += -Wall
CPP_FLAGS += -Werror
CPP_FLAGS += -Wextra
endif

CPP_FLAGS += -std=c++98
CPP_FLAGS += -g
CPP_FLAGS += -fsanitize=address

all:	build

exec:	run nav

build:	$(NAME)

run:
	./$(NAME)

nav:
	google-chrome $(CGI_OUT)

$(NAME):	$(OBJ)
	$(CC) $(CPP_FLAGS) -o $(NAME) $(OBJ)

%.o:	%.cpp
	$(CC) $(CPP_FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re: fclean all

.PHONY: clean fclean re
