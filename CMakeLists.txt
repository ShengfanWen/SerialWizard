cmake_minimum_required(VERSION 3.9)
project(SerialWizard)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# set(QT_DIR C:/Qt/Qt5.9.4/5.9.4/mingw53_32)
# set(CMAKE_PREFIX_PATH ${QT_DIR} C:/Qt/Qt5.9.4/)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Core)
get_target_property(QtCore_location Qt5::Core LOCATION)
set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE")
# link_directories(${QT_DIR}/lib C:/Qt/Qt5.9.4/Tools/mingw530_32/lib)
foreach (plugin ${Qt5Network_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    message("Plugin ${plugin} is at location ${_loc}")
endforeach ()


set(SOURCE_FILES main.cpp mainwindow.cpp mainwindow.h global.cpp global.h data/SerialReadWriter.cpp data/SerialReadWriter.h serial/SerialController.cpp serial/SerialController.h serial/NormalSerialController.cpp serial/NormalSerialController.h)
#add_executable(SerialWizard ${SOURCE_FILES})
add_executable(SerialWizard WIN32 ${SOURCE_FILES})
qt5_use_modules(SerialWizard Widgets Core SerialPort Network)
