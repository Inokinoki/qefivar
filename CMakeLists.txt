cmake_minimum_required(VERSION 3.14)

project(QEFI LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QEFIVAR_LIB)
set(QEFIVAR_INCLUDE)

find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(EFIVAR efivar)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_library(QEFI SHARED
  QEFI_global.h
  qefi.cpp
  qefi.h
)
set_target_properties(QEFI PROPERTIES PUBLIC_HEADER qefi.h)

if(EFIVAR_FOUND)
    message("Use libefivar for EFI operations")
    include_directories(QEFI ${EFIVAR_INCLUDE_DIRS})
    target_link_libraries(QEFI PRIVATE ${EFIVAR_LIBRARIES})
elseif(WIN32)
    message("Use Windows API for EFI operations")
    # TODO: Add include and lib from Windows API
else()
    message(FATAL_ERROR "No EFI utility found, please make sure you have libefivar installed")
endif()

target_link_libraries(QEFI PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(QEFI PRIVATE QEFI_LIBRARY)

install(TARGETS QEFI PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
