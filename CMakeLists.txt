cmake_minimum_required(VERSION 3.10)

project(Video LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)

set(SRC
    src/main.cpp
    src/mainwindow.cpp
    src/core/project.cpp
    src/core/session.cpp
    src/core/math_utils.cpp
    src/ui/deleteprojectdialog.cpp
    src/ui/graphselectiondialog.cpp
    src/ui/onediagraphdialog.cpp
    src/ui/twodiagraphdialog.cpp
    src/ui/graph1dwindow.cpp
    src/ui/graph2dwindow.cpp
)

set(HEADERS
    src/mainwindow.h
    src/core/project.h
    src/core/session.h
    src/core/math_utils.h
    src/ui/deleteprojectdialog.h
    src/ui/graphselectiondialog.h
    src/ui/onediagraphdialog.h
    src/ui/twodiagraphdialog.h
    src/ui/graph1dwindow.h
    src/ui/graph2dwindow.h
)

set(FORMS
    ui/MainWindow.ui
)

add_executable(${PROJECT_NAME}
    ${SRC}
    ${HEADERS}
    ${FORMS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
