cmake_minimum_required(VERSION 3.10)
project(HealthTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
find_package(OpenSSL REQUIRED)

find_library(SQLITE3_LIBRARY sqlite3)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

if (NOT SQLITE3_LIBRARY OR NOT SQLITE3_INCLUDE_DIR)
    message(FATAL_ERROR "No se encontró SQLite3. Instala libsqlite3-dev")
endif()

include_directories(
    include
    ${SQLITE3_INCLUDE_DIR}
)

set(SOURCES
    src/main.cpp
    src/User.cpp
    src/HealthRecord.cpp
    src/DatabaseManager.cpp
    src/HealthAnalyzer.cpp
    src/CSVExporter.cpp
    src/mainwindow.cpp
)

set(HEADERS
    include/User.h
    include/HealthRecord.h
    include/DatabaseManager.h
    include/HealthAnalyzer.h
    include/CSVExporter.h
    include/mainwindow.h
)

set(UI_FILES
    ui/mainwindow.ui
)

add_executable(health_app
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    Doxyfile.in
)

target_link_libraries(health_app
    Qt5::Widgets
    ${SQLITE3_LIBRARY}
    OpenSSL::Crypto
)

find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    set(PROJECT_NAME_DOXY "${PROJECT_NAME}")
    set(INPUT_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/include
         ${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
endif()

