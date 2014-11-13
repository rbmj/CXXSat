cmake_minimum_required(VERSION 2.8)

project(sat)

include_directories(/usr/lib/llvm-3.5/include ${CMAKE_CURRENT_SOURCE_DIR}/inc)
link_directories(/usr/lib/llvm-3.5/lib)
add_library(cxxsat src/Circuit.cpp src/Variable.cpp src/Argument.cpp src/Sat.cpp src/Gates.cpp src/DynVar.cpp)
add_executable(gen gen.cpp)
add_executable(CircuitTest tests/CircuitTest.cpp)
add_executable(IntegerTest tests/IntegerTest.cpp)
add_executable(DynamicTest tests/DynamicTest.cpp)
set(CMAKE_CXX_FLAGS "-g -std=c++14 -Wall")

target_link_libraries(CircuitTest cxxsat minisat)
target_link_libraries(IntegerTest cxxsat minisat)
target_link_libraries(DynamicTest cxxsat minisat)
target_link_libraries(gen clangFrontend clangSerialization clangDriver clangTooling clangParse clangSema clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangAnalysis clangRewriteFrontend clangRewriteCore clangEdit clangAST clangLex clangBasic llvm LLVM-3.5)

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

