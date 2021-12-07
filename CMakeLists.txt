cmake_minimum_required(VERSION 3.5)

project(StaticMemoryAnalyzer VERSION 1.0 LANGUAGES CXX)

add_executable(StaticMemoryAnalyzer src/StaticMapAnalyzer.cpp)

if (NOT TARGET StaticMemoryAnalyzer)
    find_package(nlohmann_json 3.10.24 REQUIRED)
endif()

add_custom_command(PRE_BUILD
    OUTPUT memory_map.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/memory_map.aurixTC38x.json.in
        $<TARGET_FILE_DIR:StaticMemoryAnalyzer>
)

add_custom_target(copyConfig ALL
    DEPENDS $<TARGET_FILE_DIR:StaticMemoryAnalyzer>/memory_map.json)

target_include_directories(StaticMemoryAnalyzer PRIVATE 
    include/
    extern/jsoncpp/single_include/nlohmann/)

target_compile_options(StaticMemoryAnalyzer PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Werror -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

# require C++17
target_compile_features(StaticMemoryAnalyzer INTERFACE cxx_std_17)

