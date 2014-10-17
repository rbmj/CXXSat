cmake_minimum_required(VERSION 2.8)

project(sat)

include_directories(/usr/lib/llvm-3.5/include)
link_directories(/usr/lib/llvm-3.5/lib)
add_executable(gen gen.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11")

target_link_libraries(gen clangFrontend clangSerialization clangDriver clangTooling clangParse clangSema clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangAnalysis clangRewriteFrontend clangRewriteCore clangEdit clangAST clangLex clangBasic llvm LLVM-3.5)

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

