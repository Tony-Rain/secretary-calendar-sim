cmake_minimum_required(VERSION 3.16)
project(secretary CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB MODEL_SOURCES src/model/*.cpp src/model/*.h)
add_library(secretary_model STATIC ${MODEL_SOURCES})
target_include_directories(secretary_model PUBLIC src/model)
target_compile_options(secretary_model PRIVATE -Wall -Wextra)

set(GUI_SOURCES
    src/main.cpp
    src/gui/ParamsDialog.cpp
    src/gui/ParamsDialog.h
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/EventDialog.cpp
    src/gui/EventDialog.h
    src/gui/ConflictDialog.cpp
    src/gui/ConflictDialog.h
    src/gui/ReportDialog.cpp
    src/gui/ReportDialog.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(secretary ${GUI_SOURCES})
else()
    add_executable(secretary ${GUI_SOURCES})
endif()

# сборка для браузера: asyncify нужен, чтобы работали диалоги через exec()
if(EMSCRIPTEN)
    target_link_options(secretary PRIVATE -sASYNCIFY -Os)
endif()

target_include_directories(secretary PRIVATE src src/gui src/model)
target_link_libraries(secretary PRIVATE secretary_model Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_options(secretary PRIVATE -Wall -Wextra)
