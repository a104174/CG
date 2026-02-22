cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(engine engine.cpp)

# macOS OpenGL/GLUT frameworks
find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
find_library(GLUT_FRAMEWORK GLUT REQUIRED)

target_link_libraries(engine PRIVATE ${OPENGL_FRAMEWORK} ${GLUT_FRAMEWORK})

# tinyxml2 (Homebrew)
find_package(tinyxml2 CONFIG REQUIRED)
target_link_libraries(engine PRIVATE tinyxml2::tinyxml2)