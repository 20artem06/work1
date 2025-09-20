
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

project(Video)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
../../../faraday/src/data
)

add_definitions(-D_AFXDLL)
# use the static MFC library
set(CMAKE_MFC_FLAG 1)


set(SRC
    ${PROJECT_SOURCE_DIR}/AviDlg.cpp
    ${PROJECT_SOURCE_DIR}/AviDlg.h
    ${PROJECT_SOURCE_DIR}/AviUtils.cpp
    ${PROJECT_SOURCE_DIR}/AviUtils.h
    ${PROJECT_SOURCE_DIR}/Child_1.cpp
    ${PROJECT_SOURCE_DIR}/Child_1.h
    ${PROJECT_SOURCE_DIR}/Child_2.cpp
    ${PROJECT_SOURCE_DIR}/Child_2.h
    ${PROJECT_SOURCE_DIR}/ColorsDlg.cpp
    ${PROJECT_SOURCE_DIR}/ColorsDlg.h
    ${PROJECT_SOURCE_DIR}/F_1_dlg.cpp
    ${PROJECT_SOURCE_DIR}/F_1_dlg.h
    ${PROJECT_SOURCE_DIR}/F_2_dlg.cpp
    ${PROJECT_SOURCE_DIR}/F_2_dlg.h
    ${PROJECT_SOURCE_DIR}/MainFrm.cpp
    ${PROJECT_SOURCE_DIR}/Mainfrm.h
    ${PROJECT_SOURCE_DIR}/resource.h
    ${PROJECT_SOURCE_DIR}/StdAfx.cpp
    ${PROJECT_SOURCE_DIR}/StdAfx.h
    ${PROJECT_SOURCE_DIR}/video.cpp
    ${PROJECT_SOURCE_DIR}/Video.h
    ${PROJECT_SOURCE_DIR}/Session.cpp
    ${PROJECT_SOURCE_DIR}/Session.h
    ${PROJECT_SOURCE_DIR}/video.rc
)

add_executable(${PROJECT_NAME} WIN32 ${SRC})


# Installation
install(TARGETS Video DESTINATION bin)
