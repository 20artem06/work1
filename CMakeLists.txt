cmake_minimum_required(VERSION 3.10)

project(Video LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Test)

set(SRC
    src/main.cpp
    src/mainwindow.cpp
    src/core/project.cpp
    src/core/session.cpp
    src/core/math_utils.cpp
    src/ui/deleteprojectdialog.cpp
    src/ui/graphselectiondialog.cpp
    src/ui/graph1dconfigdialog.cpp
    src/ui/graph2dconfigdialog.cpp
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
    src/ui/graph1dconfigdialog.h
    src/ui/graph2dconfigdialog.h
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

enable_testing()

set(TEST_SRC
    tests/test_project.cpp
    tests/test_session.cpp
)

add_executable(video_tests
    ${TEST_SRC}
    src/core/project.cpp
    src/core/session.cpp
    src/core/math_utils.cpp
)

target_include_directories(video_tests PRIVATE src)
target_link_libraries(video_tests PRIVATE Qt5::Widgets Qt5::Test)

add_test(NAME video_unit_tests COMMAND video_tests)
