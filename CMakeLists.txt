####################################################
# CMake is a build file generator.
# It does not compile your software, but
# rather uses the CMake Language to 
# figure out how to generate another file
# for which you can then use to build projects.
# CMake is a specification language for build tools.
####################################################

############### INSTALLATION #######################
# Installation on Mac
# 'brew install cmake'
#
# Installation on Linux
# 'sudo apt-get install cmake'
#
# Installation on Windows
# See download page and download
####################################################

############## How to use CMake ####################
# I recommend you create a separate directory for
# your CMake generated files. This is a clean way
# to separate out all of your binary files generated
# from the source code. An example on Linux is
# the following:
#
# mkdir bin	# Make a directory
# cd bin	# navigate to that directory
# cmake .. 	# Then execute the CMakeLists.txt
#		# file found one directory up.
####################################################

####################################################
# Writing a CMake file involves writing one 
# cmake command per line.

# The minimum required version of CMake to use
cmake_minimum_required(VERSION 3.10)

# Set the project details
project(
    App             # Name of our application
    VERSION 1.0     # Version of our software
    LANGUAGES CXX)  # Language that we are using

# Set language standard for C++
set(CMAKE_CXX_STANDARD 17)

# Where are the include directories
include_directories("./include/")
include_directories("/user/include/")

# Where are the libraries
link_directories("/usr/lib/x86_64-linux-gnu/")

# Add the source code files to a specific project
add_executable(App ./src/App.cpp
                   ./src/Command.cpp ./src/Draw.cpp ./src/Paint.cpp ./src/Clear.cpp
                   ./src/App_Factory.cpp ./src/main.cpp)

add_executable(App_test ./src/App.cpp
        ./src/Command.cpp ./src/Draw.cpp ./src/Paint.cpp ./src/Clear.cpp
        ./src/App_Factory.cpp
        ./tests/main_test.cpp ./tests/catch_amalgamated.cpp)

# Add any libraries
target_link_libraries(App -lpthread sfml-graphics sfml-window sfml-system sfml-network -lGL)

target_link_libraries(App_test -lpthread sfml-graphics sfml-window sfml-system sfml-network -lGL)
