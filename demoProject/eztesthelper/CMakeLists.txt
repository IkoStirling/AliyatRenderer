set(PKG_CONFIG_EXECUTABLE "D:/Aliyat/vcpkg/installed/x64-windows/tools/pkgconf/pkgconf.exe")

cmake_minimum_required(VERSION 3.12)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widget)

find_package(PkgConfig)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE app_src ./main.cpp)
file(GLOB_RECURSE filepkg src/ *.cpp)
file(GLOB_RECURSE qrcpkg ./ *.qrc)
file(GLOB_RECURSE uipkg ./ *.ui)

add_library(qt_ui STATIC ${uipkg} ${qrcpkg} ${filepkg})

target_include_directories(qt_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(qt_ui Qt6::Core Qt6::Gui Qt6::Widgets)
target_link_libraries(qt_ui
	PRIVATE
	PkgConfig::POPPLER_CPP
)

add_executable(eztesthelper ${app_src})
target_link_libraries(eztesthelper PRIVATE qt_ui)
