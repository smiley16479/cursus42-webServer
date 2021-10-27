#!/bin/bash

g++ main.cpp config_checker.cpp server.cpp -std=c++98 -fsanitize=address -g3 -D_debug_ && ./a.out
