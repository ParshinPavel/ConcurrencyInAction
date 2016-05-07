cmake_minimum_required(VERSION 3.5)
project(ConcurrencyInAction)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_subdirectory(Listing2.7)
add_subdirectory(Listing2.8)
add_subdirectory(Listing3.1)
add_subdirectory(Listing3.5)
add_subdirectory(Listing3.8)
add_subdirectory(Listing3.12)
add_subdirectory(Listing4.5)
add_subdirectory(packaged_task)
add_subdirectory(Listing4.12)