cmake_minimum_required(VERSION 3.16)
project(AdvancedCalculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(AdvancedCalculator
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    historymanager.cpp
    historymanager.h
    calculator.cpp
    calculator.h
)

target_link_libraries(AdvancedCalculator Qt6::Widgets)
