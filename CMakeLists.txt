cmake_minimum_required(VERSION 3.5.0)
project(CACHES VERSION 0.1.0 LANGUAGES C CXX)

add_executable(CACHES main.cpp perfect_cache.hpp LFU_cache.hpp settings_parser.hpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
