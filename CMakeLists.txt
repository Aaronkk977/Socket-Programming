cmake_minimum_required(VERSION 3.14)
project(cn2025_chat CXX)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -O2)
include_directories(include)

add_executable(server
  src/server/main_server.cpp src/server/server_core.cpp
  src/common/proto.cpp src/common/net.cpp src/common/util.cpp)

add_executable(client
  src/client/main_client.cpp
  src/client/cli_shell.cpp
  src/common/proto.cpp
  src/common/net.cpp 
  src/common/util.cpp)
