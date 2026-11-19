cmake_minimum_required(VERSION 3.16)
project(PDFBookmarkViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Poppler — напрямую из vcpkg
set(POPPLER_DIR D:/yurem/vcpkg/installed/x64-windows)
find_path(POPPLER_CPP_INCLUDE_DIR poppler/cpp/poppler-document.h
    PATHS ${POPPLER_DIR}/include
)
find_path(POPPLER_QT6_INCLUDE_DIR poppler-qt6.h
    PATHS ${POPPLER_DIR}/include/poppler/qt6
)

find_library(POPPLER_CPP_LIB
    NAMES poppler-cpp
    PATHS ${POPPLER_DIR}/debug/lib ${POPPLER_DIR}/lib
)
find_library(POPPLER_QT6_LIB
    NAMES poppler-qt6
    PATHS ${POPPLER_DIR}/debug/lib ${POPPLER_DIR}/lib
)

if(NOT POPPLER_CPP_INCLUDE_DIR OR NOT POPPLER_QT6_INCLUDE_DIR OR
   NOT POPPLER_CPP_LIB OR NOT POPPLER_QT6_LIB)
    message(FATAL_ERROR "Poppler not found. Check ${POPPLER_DIR}")
endif()

message(STATUS "Poppler cpp: include=${POPPLER_CPP_INCLUDE_DIR}, lib=${POPPLER_CPP_LIB}")
message(STATUS "Poppler qt6: include=${POPPLER_QT6_INCLUDE_DIR}, lib=${POPPLER_QT6_LIB}")

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
    src/pdfrenderer.h src/pdfrenderer.cpp
    src/bookmarkmanager.h src/bookmarkmanager.cpp
    src/continuousview.h src/continuousview.cpp
    src/pdfeditor.h src/pdfeditor.cpp
    src/thumbnailwidget.h src/thumbnailwidget.cpp
    src/bookmarkwidget.h src/bookmarkwidget.cpp
    resources/icons.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${POPPLER_CPP_INCLUDE_DIR}
    ${POPPLER_QT6_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        ${POPPLER_CPP_LIB}
        ${POPPLER_QT6_LIB}
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Post-build: копирование DLL из Qt SDK для Release
# ОТКЛЮЧАЕМ vcpkg post-build, который перезаписывает DLL
set(VCPKG_APPLOCAL_DEPS OFF)

# Копируем DLL из Qt SDK и poppler из vcpkg
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/Qt/6.12.0/msvc2022_64/bin/Qt6Core.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/Qt/6.12.0/msvc2022_64/bin/Qt6Gui.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/Qt/6.12.0/msvc2022_64/bin/Qt6Widgets.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/yurem/vcpkg/installed/x64-windows/bin/qwindows.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
    COMMENT "Copying platform and Qt DLLs"
)
