cmake_minimum_required(VERSION 3.16)

# This sets your project name to notedio
project(notedio VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

add_subdirectory(whisper.cpp EXCLUDE_FROM_ALL)
set(PROJECT_SOURCES
    # UI Files (Inside voiceInput/ folder)
    voiceInput/main.cpp
    voiceInput/mainwindow.cpp
    voiceInput/mainwindow.h
    voiceInput/mainwindow.ui

    # # Database Files (Uncomment and update when ready)
    # database/database.cpp
    # database/database.h

    # # Whisper.cpp Files (Uncomment and update when ready)
    # whispercpp/whisper.cpp
    # whispercpp/whisper.h

    # # Llama Files (Uncomment and update when ready)
    # llama/llama.cpp
    # llama/llama.h
)

include_directories(
    voiceInput
    database
    whispercpp
    llama
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    # Changed executable target name to notedio
    qt_add_executable(notedio
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(notedio SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(notedio
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Changed target link libraries to notedio
target_link_libraries(notedio PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.notedio)
endif()

# Changed properties target to notedio
set_target_properties(notedio PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
# Changed install target to notedio
install(TARGETS notedio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    # Changed finalization target to notedio
    qt_finalize_executable(notedio)
endif()