cmake_minimum_required(VERSION 3.14)
project(tp4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Source files
set(COMMON_SOURCES
  common/tcp_connection.cpp
  common/rpc_protocol.cpp
)

add_executable(
  agent
  agent.cpp
  ${COMMON_SOURCES}
)

target_include_directories(
  agent PRIVATE ${CMAKE_SOURCE_DIR}
)

target_link_libraries(
  agent
  Threads::Threads
)


