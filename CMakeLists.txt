cmake_minimum_required(VERSION 3.21)
project(chess C)

set(CMAKE_C_STANDARD 11)

add_executable(chess main.c chess.c chess.h)
