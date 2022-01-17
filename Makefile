SRC_DIR = srcs

OBJ_DIR = obj

SRC_NAME += main.cpp
SRC_NAME += cgi_handler.cpp
SRC_NAME += config_checker.cpp
SRC_NAME += server.cpp
SRC_NAME += request_handler.cpp
SRC_NAME += client_handler.cpp
SRC_NAME += client_info.cpp

SRC = $(addprefix $(SRC_DIR)/,$(SRC_NAME))

OBJ_NAME = $(SRC_NAME:.cpp=.o)

OBJ = $(addprefix $(OBJ_DIR)/,$(OBJ_NAME))

CC = clang++

RM = rm -f

NAME = webserv

INC = -I inc

#####MULTITHREADED BUILD FLAGS######
#MAKEFLAGS+=-j8

CGI_OUT = "http://127.0.0.1:8081/scripts/layout.html"

#CPP_FLAGS += -g
CPP_FLAGS += -Wall
CPP_FLAGS += -Werror
CPP_FLAGS += -Wextra
CPP_FLAGS += -std=c++98

ULIMIT = $(shell ulimit -s)

MAX_LEN = $(ULIMIT)

#Useless test, MAX_LEN should just be set to ulimit -s
#ifeq ($(shell test $(MAX_LEN) -gt $(ULIMIT); echo $$), 0)
#MAX_LEN = $(ULIMIT)
#else
#MAX_LEN = $(ULIMIT)
#endif

SET_LEN = -D MAX_LEN=$(MAX_LEN)

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
	$(CC) $(CPP_FLAGS) $(SET_LEN) -o $(NAME) $(OBJ) $(INC)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(INC) $(SET_LEN) $(CPP_FLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean:	clean
	$(RM) $(NAME)

re: fclean all

.PHONY: clean fclean re
