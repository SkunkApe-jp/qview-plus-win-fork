cmake_minimum_required(VERSION 3.16)

project(QtImageViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(qt_image_viewer
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/imageviewer.cpp
    src/imageviewer.h
    src/imagemodel.cpp
    src/imagemodel.h
)

target_link_libraries(qt_image_viewer PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
