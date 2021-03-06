cmake_minimum_required(VERSION 2.8)

project(sat)

include_directories(/usr/lib/llvm-3.5/include ${CMAKE_CURRENT_SOURCE_DIR}/inc)
link_directories(/usr/lib/llvm-3.5/lib)

add_library(cxxsat SHARED
    src/lib/Circuit.cpp
    src/lib/Variable.cpp
    src/lib/Argument.cpp
    src/lib/Sat.cpp
    src/lib/Gates.cpp
    src/lib/FlexInt.cpp
    src/lib/TypeInfo.cpp
)
add_executable(plugin
    src/cxxsat.cpp
    src/Scope.cpp
    src/VarRef.cpp
)

set_target_properties(plugin PROPERTIES OUTPUT_NAME cxxsat)

add_executable(FlexIntTest tests/FlexIntTest.cpp)
add_executable(CircuitTest tests/CircuitTest.cpp)
add_executable(IntegerTest tests/IntegerTest.cpp)
add_executable(FactorTest tests/FactorTest.cpp)

set(CMAKE_CXX_FLAGS "-O2 -g -std=c++14 -Wall")

target_link_libraries(FlexIntTest cxxsat minisat)
target_link_libraries(CircuitTest cxxsat minisat)
target_link_libraries(IntegerTest cxxsat minisat)
target_link_libraries(FactorTest cxxsat minisat)

target_link_libraries(plugin clangFrontend clangSerialization clangDriver clangTooling clangParse clangSema clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangAnalysis clangRewriteFrontend clangRewrite clangEdit clangAST clangLex clangBasic llvm LLVM-3.5 cxxsat minisat)

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS)

install(DIRECTORY inc/CXXSat DESTINATION include)
install(TARGETS cxxsat plugin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
