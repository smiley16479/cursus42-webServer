#!/bin/bash

# -std=c++98
FLAG="-Wall -Werror -Wextra"

if [ $1 = "t" ]; then
	echo "run avec configuration_files/valid/test.conf"
    TEST="configuration_files/valid/test.conf"
else
	echo "run avec configuration_files/valid/simple.conf"
    TEST="configuration_files/valid/simple.conf"
fi

SRC="main.cpp config_checker.cpp server.cpp request_handler.cpp client_handler.cpp" # cgi_handler.cpp "
g++ $SRC -fsanitize=address -g3 -D_debug_ -D_log_  #&& ./a.out $TEST
# g++ $SRC -D_log_ && ./a.out $TEST