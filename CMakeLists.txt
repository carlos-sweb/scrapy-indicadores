cmake_minimum_required(VERSION 3.16)
project(indicadores LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
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
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED )

# 2. Configurar rutas dinámicas para CPR
if(IS_TERMUX)
    # Rutas para Termux (Android)
    set(CPR_INCLUDE_DIR "$ENV{HOME}/cpr/include")
    set(CPR_LIBRARY "$ENV{HOME}/cpr/build/lib/libcpr.a")
    set(LEXBOR_STATIC "$ENV{HOME}/lexbor/build/liblexbor_static.a")
    set(LEXBOR_INCLUDE_DIR "$ENV{HOME}/lexbor/source")
    set(FMT_STATIC "$ENV{HOME}/fmt/libfmt.a")
    set(INSTALL_BIN_DIR "$ENV{HOME}/bin" CACHE PATH "Binary install directory for Termux")    
else()
    # Rutas para Linux estándar
    set(CPR_INCLUDE_DIR "/usr/local/include/cpr")
    set(CPR_LIBRARY "/usr/local/lib64/libcpr.a")
    set(LEXBOR_STATIC "/usr/local/lib64/liblexbor_static.a")
    set(LEXBOR_INCLUDE_DIR "/usr/local/include")
    set(FMT_STATIC "/usr/local/lib64/libfmt.a")
    set(INSTALL_BIN_DIR "/opt/${PROJECT_NAME}/bin" CACHE PATH "Binary install directory for Linux")   
endif()

add_compile_definitions(INSTALL_BIN_DIR="${INSTALL_BIN_DIR}")

add_executable(
    indicadores
    src/main.cpp
)

target_link_libraries(indicadores PRIVATE 
    curl 
    ssl 
    crypto 
    z 
    ${FMT_STATIC}
    ${LEXBOR_STATIC} 
    ${CPR_LIBRARY}
)

set_target_properties(indicadores PROPERTIES
    INSTALL_RPATH "/usr/local/lib64"
    BUILD_WITH_INSTALL_RPATH TRUE
)
include_directories(includes ${CPR_INCLUDE_DIR} ${LEXBOR_INCLUDE_DIR})
include(GNUInstallDirs)

#install(TARGETS indicadores
#    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
#)

install(TARGETS indicadores
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
