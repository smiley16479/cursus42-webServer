SRC += main.cpp
SRC += cgi_handler.cpp
SRC += config_checker.cpp
SRC += server.cpp
SRC += request_handler.cpp
SRC += client_handler.cpp

OBJ = $(SRC:.cpp=.o)

CC = clang++

RM = rm -f

NAME = webserv

#####MULTITHREADED BUILD FLAGS######
#MAKEFLAGS+=-j8

CGI_OUT = "http://127.0.0.1:8081/scripts/layout.html"

CPP_FLAGS += -g
CPP_FLAGS += -Wall
CPP_FLAGS += -Werror
CPP_FLAGS += -Wextra
CPP_FLAGS += -std=c++98

ifeq ($(d), 0)
CPP_FLAGS += -fsanitize=address
endif

all:	build

exec:	build run nav

build:	$(NAME)

run:
	./$(NAME) &

nav: run
	firefox --private-window $(CGI_OUT)

goo-nav:
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
