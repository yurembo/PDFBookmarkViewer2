cmake_minimum_required(VERSION 3.16)
project(PDFBookmarkViewer2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Pdf PdfWidgets)

# Poppler — только для чтения outline и метаданных PDF
find_path(POPPLER_CPP_INCLUDE_DIR poppler/cpp/poppler-document.h
    PATHS /usr/include
)
find_path(POPPLER_QT6_INCLUDE_DIR poppler/qt6/poppler-qt6.h
    PATHS /usr/include
)
find_library(POPPLER_CPP_LIB
    NAMES poppler-cpp
    PATHS /usr/lib/x86_64-linux-gnu
)
find_library(POPPLER_QT6_LIB
    NAMES poppler-qt6
    PATHS /usr/lib/x86_64-linux-gnu
)

if(NOT POPPLER_CPP_INCLUDE_DIR OR NOT POPPLER_QT6_INCLUDE_DIR OR
   NOT POPPLER_CPP_LIB OR NOT POPPLER_QT6_LIB)
    message(FATAL_ERROR "Poppler not found. Install: sudo apt install libpoppler-cpp-dev libpoppler-qt6-dev")
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
    src/pdfrenderer.h src/pdfrenderer.cpp
    src/continuousview.h src/continuousview.cpp
    src/bookmarkmanager.h src/bookmarkmanager.cpp
    src/bookmarkwidget.h src/bookmarkwidget.cpp
    resources/icons.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${POPPLER_CPP_INCLUDE_DIR}
    ${POPPLER_QT6_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        Qt6::Pdf Qt6::PdfWidgets
        ${POPPLER_CPP_LIB} ${POPPLER_QT6_LIB}
)
