cmake_minimum_required(VERSION 3.10.0)
project(ticker_loader LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("C:/msys64/ucrt64/include")
link_directories("C:/msys64/ucrt64/lib")

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# needed to force proper python3 on my system
# NEEDS TO CHANGE
set(Python3_EXECUTABLE "C:/Users/Michi/AppData/Local/Microsoft/WindowsApps/python3.exe")

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Python3 Executable: ${Python3_EXECUTABLE}")
message(STATUS "pybind11 DIR: '${pybind11_DIR}'")

# Notice 'CONFIG' and 'HINTS'
find_package(pybind11 CONFIG REQUIRED HINTS "${pybind11_DIR}")

pybind11_add_module(ticker_loader 
    pybind_wrapper.cpp 
    ticker_loader.c
)

target_link_libraries(ticker_loader PRIVATE curl cjson)