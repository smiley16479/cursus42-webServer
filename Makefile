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

CPP_FLAGS += -Wall
CPP_FLAGS += -Werror
CPP_FLAGS += -Wextra
CPP_FLAGS += -std=c++98

ULIMIT = $(shell expr $(shell ulimit -s) '*' 1000)

MAX_LEN = $(ULIMIT)

SET_LEN = -D MAX_LEN=$(MAX_LEN)

#Pernet d'activer/ desactiver les flags
#Il faut ajouter -d=$ a la fin de la ligne make pour activer les options voulues
#ex: "make re d=1" (recompile avec "-D _debug_")
ifeq ($(d), 0)
CPP_FLAGS += -g
CPP_FLAGS += -fsanitize=address
endif
ifeq ($(d), 1)
CPP_FLAGS += -D _debug_ # Toggle les sorties de debug par le preproccesseur 
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
