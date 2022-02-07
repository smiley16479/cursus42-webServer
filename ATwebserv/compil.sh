#!/bin/bash

# -std=c++98
FLAG="-Wall -Werror -Wextra"
SRC="main.cpp config_checker.cpp server.cpp request_handler.cpp client_handler.cpp" # cgi_handler.cpp "
g++ $SRC -fsanitize=address -g3 -D_debug_ -D_log_ && ./a.out
# g++ $SRC -D_log_ && ./a.out
