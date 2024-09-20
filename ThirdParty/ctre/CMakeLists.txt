cmake_minimum_required(VERSION 3.14.0)

project(ctre
  HOMEPAGE_URL "https://compile-time.re"
  VERSION 3.9.0
  LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Fast compile-time regular expressions with support for matching/searching/capturing during compile-time or runtime.")

add_library(ctre INTERFACE)
target_include_directories(ctre
	INTERFACE
		include
)
