#!/bin/bash

# -std=c++98
# -Wall -Werror -Wextra
g++ -Wall -Werror -Wextra main.cpp config_checker.cpp server.cpp request_handler.cpp client_handler.cpp cgi_handler.cpp -fsanitize=address -g3 -D_debug_ && ./a.out
