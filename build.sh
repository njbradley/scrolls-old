#!/bin/bash

g++ main.cc -o main $* -std=c++14 -lglfw -framework CoreVideo -framework OpenGL -framework IOKit -lGLEW
