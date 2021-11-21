#!/bin/bash

g++ main.cpp config_checker.cpp server.cpp header_handler.cpp  -fsanitize=address -g3 -D_debug_ && ./a.out
