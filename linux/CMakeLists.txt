cmake_minimum_required(VERSION 3.10)
project(tmini_plus_emulator_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tmini_plus_emulator
  src/main.cpp
  src/utils.cpp
)

target_link_libraries(tmini_plus_emulator pthread)
target_include_directories(tmini_plus_emulator PRIVATE include)
