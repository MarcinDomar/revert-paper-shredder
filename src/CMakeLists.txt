cmake_minimum_required(VERSION 3.5)

project(revert-paper-shredder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(revert-paper-shredder main.cpp SolutionFinder.cpp PrefixTree.cpp)
