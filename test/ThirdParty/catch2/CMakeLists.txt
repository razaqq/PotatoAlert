cmake_minimum_required(VERSION 3.0)

project(catch2)

add_library(Catch2 INTERFACE)
add_library(Catch2::Test ALIAS Catch2)
target_include_directories(Catch2 INTERFACE include)
