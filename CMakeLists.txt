cmake_minimum_required(VERSION 3.22)

project(PDFBookmarkViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 path (6.12.0 has full MSVC support)
set(QT6_PATH "D:/Qt/6.12.0/msvc2022_64")
set(CMAKE_PREFIX_PATH "${QT6_PATH}")

# Poppler from vcpkg (x64-windows = MSVC)
set(POPPLER_ROOT "D:/yurem/vcpkg/packages/poppler_x64-windows")

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Pdf PdfWidgets)

# Find Poppler
find_path(POPPLER_CPP_INCLUDE_DIR poppler/cpp/poppler-document.h
    PATHS "${POPPLER_ROOT}/include"
)
find_path(POPPLER_QT6_INCLUDE_DIR poppler/qt6/poppler-qt6.h
    PATHS "${POPPLER_ROOT}/include"
)

if(NOT POPPLER_CPP_INCLUDE_DIR OR NOT POPPLER_QT6_INCLUDE_DIR)
    message(FATAL_ERROR "Poppler headers not found")
endif()

add_executable(${PROJECT_NAME} WIN32
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
    "${POPPLER_ROOT}/lib/poppler-cpp.lib"
    "${POPPLER_ROOT}/lib/poppler-qt6.lib"
)

# ---- Copy Qt DLLs from Qt 6.12.0 (NOT from vcpkg) ----
set(QT6_DEPLOY_BIN "${QT6_PATH}/bin")
set(QT6_DEPLOY_PLUGINS "${QT6_PATH}/plugins")
set(QT6_DEPLOY_QML "${QT6_PATH}/qml")
set(QT6_OUTPUT "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Core Qt DLLs
foreach(qtlib IN LISTS QT6_FOUND_IMPLIES)
endforeach()

# Manually list needed Qt DLLs
set(QT6_REQUIRED_DLLS
    "${QT6_DEPLOY_BIN}/Qt6Core.dll"
    "${QT6_DEPLOY_BIN}/Qt6Gui.dll"
    "${QT6_DEPLOY_BIN}/Qt6Widgets.dll"
    "${QT6_DEPLOY_BIN}/Qt6Pdf.dll"
    "${QT6_DEPLOY_BIN}/Qt6PdfWidgets.dll"
    "${QT6_DEPLOY_BIN}/Qt6Network.dll"
    "${QT6_DEPLOY_BIN}/Qt6Concurrent.dll"
)

foreach(dll IN LISTS QT6_REQUIRED_DLLS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll}"
        "${QT6_OUTPUT}"
    )
endforeach()

# Platform plugin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QT6_OUTPUT}/platforms"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${QT6_DEPLOY_PLUGINS}/platforms/qwindows.dll"
    "${QT6_OUTPUT}/platforms/"
)

# Image formats plugins
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QT6_OUTPUT}/imageformats"
)
file(GLOB IMAGE_PLUGINS "${QT6_DEPLOY_PLUGINS}/imageformats/*.dll")
foreach(plugin IN LISTS IMAGE_PLUGINS)
    get_filename_component(plugin_name "${plugin}" NAME)
    string(FIND "${plugin_name}" "d.dll" is_debug)
    if(${is_debug} EQUAL -1)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${plugin}"
            "${QT6_OUTPUT}/imageformats/"
        )
    endif()
endforeach()

# ---- Copy Poppler DLLs ----
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${POPPLER_ROOT}/bin/poppler-cpp.dll"
    "${QT6_OUTPUT}"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${POPPLER_ROOT}/bin/poppler-qt6.dll"
    "${QT6_OUTPUT}"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${POPPLER_ROOT}/bin/poppler.dll"
    "${QT6_OUTPUT}"
)
