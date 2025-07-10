cmake_minimum_required(VERSION 3.16)
project(indicadores LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Detectar si estamos en Termux (Android)
if(EXISTS "/data/data/com.termux/files/usr")
    set(IS_TERMUX TRUE)
    message(STATUS "Entorno detectado: Termux (Android)")
else()
    set(IS_TERMUX FALSE)
    message(STATUS "Entorno detectado: Sistema Linux estándar")
endif()

#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED )
find_package(cpr REQUIRED)

find_library(LEXBOR_STATIC liblexbor_static.a PATHS /usr/local/lib64 REQUIRED)

# 2. Configurar rutas dinámicas para CPR
if(IS_TERMUX)
    # Rutas para Termux (Android)
    set(CPR_INCLUDE_DIR "$ENV{HOME}/cpr/include")
    set(CPR_LIBRARY "$ENV{HOME}/cpr/build/lib/libcpr.a")
    set(LEXBOR_STATIC "$ENV{HOME}/lexbor/build/liblexbor_static.a")
else()
    # Rutas para Linux estándar
    set(CPR_INCLUDE_DIR "/usr/local/include/cpr")
    set(CPR_LIBRARY "/usr/local/lib64/libcpr.a")
    set(LEXBOR_STATIC "/usr/local/lib64/liblexbor_static.a")
endif()

#find_library(CPR_STATIC libcpr.a PATHS /usr/local/lib64 REQUIRED)

add_executable(
    indicadores
    src/main.cpp 
    src/ada.cpp
)

target_link_libraries(indicadores PRIVATE 
    curl 
    ssl 
    crypto 
    z 
    ${LEXBOR_STATIC} 
    ${CPR_LIBRARY}
)

set_target_properties(indicadores PROPERTIES
    INSTALL_RPATH "/usr/local/lib64"
    BUILD_WITH_INSTALL_RPATH TRUE
)
include_directories(includes ${CPR_INCLUDE_DIR})
include(GNUInstallDirs)
install(TARGETS indicadores
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)