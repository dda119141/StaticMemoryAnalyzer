cmake_minimum_required(VERSION 3.5)

project(StaticMemoryAnalyzer VERSION 1.0 LANGUAGES CXX)

#find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(StaticMemoryAnalyzer src/StaticMapAnalyzer.cpp)

target_include_directories(StaticMemoryAnalyzer PRIVATE include/)

target_compile_options(StaticMemoryAnalyzer PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Werror -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

# require C++17
target_compile_features(StaticMemoryAnalyzer INTERFACE cxx_std_17)

