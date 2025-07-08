cmake_minimum_required(VERSION 3.16)
project(indicadores LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static")

find_package(PkgConfig REQUIRED)
find_package(cpr REQUIRED)
find_package(OpenSSL REQUIRED )
find_package(ada REQUIRED)

add_executable(indicadores src/main.cpp)
target_link_libraries(indicadores PRIVATE cpr::cpr lexbor ada::ada)
#target_link_libraries(indicadores PRIVATE cpr::cpr ada::ada)
set_target_properties(indicadores PROPERTIES
    INSTALL_RPATH "/usr/local/lib64"
    BUILD_WITH_INSTALL_RPATH TRUE
)
include(GNUInstallDirs)
install(TARGETS indicadores
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)