cmake_minimum_required(VERSION 3.28)
project(TwoDFractals)

set(CMAKE_CXX_STANDARD 26)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package (Eigen3 3.4 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp
        src/range.h
        src/logisticMap.h
        src/grid.h)

target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system Eigen3::Eigen)
