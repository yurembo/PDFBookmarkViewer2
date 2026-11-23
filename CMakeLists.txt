cmake_minimum_required(VERSION 3.22)

project(PDFBookmarkViewer VERSION 1.0.0 LANGUAGES CXX)

# === C++ Standard ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === Qt Auto-Tools ===
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# === Qt6 Path ===
set(QT6_PATH "D:/Qt/6.12.0/msvc2022_64")
set(CMAKE_PREFIX_PATH "${QT6_PATH}")

# === Find Qt6 ===
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Pdf PdfWidgets)

# === Find Poppler via vcpkg ===
# When using vcpkg toolchain, poppler can be found via CMake config or pkg-config
set(POPPLER_ROOT "D:/vcpkg/installed/x64-windows")

find_path(POPPLER_CPP_INCLUDE_DIR
    NAMES poppler/cpp/poppler-document.h
    PATHS "${POPPLER_ROOT}/include"
)

find_library(POPPLER_CPP_LIB
    NAMES poppler-cpp
    PATHS "${POPPLER_ROOT}/lib"
)

# === Check Poppler ===
if(POPPLER_CPP_INCLUDE_DIR AND POPPLER_CPP_LIB)
    message(STATUS "Poppler found: lib=${POPPLER_CPP_LIB}")
    message(STATUS "Poppler includes: ${POPPLER_CPP_INCLUDE_DIR}")
    set(HAVE_POPPLER ON)
else()
    message(WARNING
        "Poppler not found!\n"
        "Installed at: ${POPPLER_ROOT}\n"
        "Please ensure Poppler is installed:\n"
        "  cd D:/vcpkg && .\\vcpkg install poppler:x64-windows"
    )
    set(HAVE_POPPLER OFF)
endif()

# === Executable ===
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
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Pdf Qt6::PdfWidgets
)

if(HAVE_POPPLER)
    target_link_libraries(${PROJECT_NAME} PRIVATE "${POPPLER_CPP_LIB}")
endif()

# === Post-build: Deploy Qt DLLs ===
set(QT6_BIN_DIR "${QT6_PATH}/bin")
set(QT6_PLUGINS_DIR "${QT6_PATH}/plugins")
set(QT6_OUTPUT_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

# Required Qt DLLs
set(QT6_DLLS
    "${QT6_BIN_DIR}/Qt6Core.dll"
    "${QT6_BIN_DIR}/Qt6Gui.dll"
    "${QT6_BIN_DIR}/Qt6Widgets.dll"
    "${QT6_BIN_DIR}/Qt6Pdf.dll"
    "${QT6_BIN_DIR}/Qt6PdfWidgets.dll"
    "${QT6_BIN_DIR}/Qt6Network.dll"
)

foreach(dll IN LISTS QT6_DLLS)
    if(EXISTS "${dll}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll}"
            "${QT6_OUTPUT_DIR}"
        )
    else()
        message(STATUS "Qt DLL not found (skipping): ${dll}")
    endif()
endforeach()

# Platform plugin
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QT6_OUTPUT_DIR}/platforms"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${QT6_PLUGINS_DIR}/platforms/qwindows.dll"
    "${QT6_OUTPUT_DIR}/platforms/"
)

# Image format plugins
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QT6_OUTPUT_DIR}/imageformats"
)
file(GLOB IMAGE_PLUGINS "${QT6_PLUGINS_DIR}/imageformats/*.dll")
foreach(plugin IN LISTS IMAGE_PLUGINS)
    get_filename_component(plugin_name "${plugin}" NAME)
    if(NOT plugin_name MATCHES "d\\.dll$")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${plugin}"
            "${QT6_OUTPUT_DIR}/imageformats/"
        )
    endif()
endforeach()

# === Post-build: Deploy Poppler DLLs and dependencies ===
if(HAVE_POPPLER AND EXISTS "${POPPLER_ROOT}/bin")
    # Все DLL из vcpkg bin, начинающиеся на poppler, libjpeg, libtiff, zlib, libwebp и т.д.
    file(GLOB POPPLER_BIN_DLLS "${POPPLER_ROOT}/bin/*.dll")
    
    foreach(dll_path IN LISTS POPPLER_BIN_DLLS)
        get_filename_component(dll_name "${dll_path}" NAME)
        # Пропускаем debug DLL (содержат "d.dll" или "d.pdb")
        if(dll_name MATCHES "d\\.dll$" OR dll_name MATCHES "d\\.pdb$")
            continue()
        endif()
        
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll_path}"
            "${QT6_OUTPUT_DIR}"
            COMMENT "Copying dependency: ${dll_name}"
        )
    endforeach()
endif()
