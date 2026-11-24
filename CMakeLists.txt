cmake_minimum_required(VERSION 3.22)

project(PDFBookmarkViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# macOS Bundle settings
set(MACOSX_BUNDLE_ICON_DIR "")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")

# Set Qt6 and Poppler paths for macOS (Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
set(POPPLER_ROOT "/opt/homebrew/opt/poppler-qt6")

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Pdf PdfWidgets)

# Find Poppler
find_path(POPPLER_CPP_INCLUDE_DIR poppler/cpp/poppler-document.h
    PATHS "${POPPLER_ROOT}/include"
          "/opt/homebrew/opt/poppler/include"
)
find_path(POPPLER_QT6_INCLUDE_DIR poppler/qt6/poppler-qt6.h
    PATHS "${POPPLER_ROOT}/include"
)

if(NOT POPPLER_CPP_INCLUDE_DIR OR NOT POPPLER_QT6_INCLUDE_DIR)
    message(FATAL_ERROR "Poppler headers not found")
endif()

# Create as macOS Bundle
add_executable(${PROJECT_NAME} MACOSX_BUNDLE
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

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Pdf Qt6::PdfWidgets
    "${POPPLER_ROOT}/lib/libpoppler-cpp.dylib"
    "${POPPLER_ROOT}/lib/libpoppler-qt6.dylib"
)

# Set rpath to find frameworks at runtime
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "PDF Bookmark Viewer"
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
    BUILD_RPATH "@executable_path/../Frameworks"
)

# Post-build: copy dylibs, fix install names, sign bundle
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/setup_bundle.sh
        "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>"
        "$<TARGET_FILE:${PROJECT_NAME}>"
    COMMENT "Copying Poppler frameworks, fixing install names, and signing bundle"
    VERBATIM
)
