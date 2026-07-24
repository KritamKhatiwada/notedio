cmake_minimum_required(VERSION 3.16)
project(notedio VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Sql)

set(GGML_NATIVE OFF CACHE BOOL "" FORCE)
set(GGML_AVX512 OFF CACHE BOOL "" FORCE)
set(GGML_AVX512_VBMI OFF CACHE BOOL "" FORCE)
set(GGML_AVX512_VNNI OFF CACHE BOOL "" FORCE)


add_subdirectory(whisper.cpp)
set(PROJECT_SOURCES
    voiceInput/main.cpp
    voiceInput/mainwindow.cpp
    voiceInput/mainwindow.h
    voiceInput/mainwindow.ui
    app/appwindow.cpp
    app/appwindow.h
    ui/homepage.cpp
    ui/homepage.h
    ui/transcriptioneditor.cpp
    ui/transcriptioneditor.h
    widgets/notecard.cpp
    widgets/notecard.h
    recording/chunkrecorder.cpp
    recording/chunkrecorder.h
    recording/transcriptionworker.cpp
    recording/transcriptionworker.h
    recording/transcriptionqueue.cpp
    recording/transcriptionqueue.h
    recording/recordingsession.cpp
    recording/recordingsession.h
    recording/whisperlocator.cpp
    recording/whisperlocator.h
    database/database.cpp
    database/database.h
)
include_directories(
    voiceInput
    app
    ui
    widgets
    recording
    database
    llama
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
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
target_link_libraries(notedio PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Sql
)
target_compile_definitions(notedio PRIVATE
    NOTEDIO_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
if(TARGET whisper-cli)
    add_dependencies(notedio whisper-cli)
    message(STATUS "notedio will build whisper-cli as a dependency")
else()
    message(WARNING "whisper-cli target not found - check whisper.cpp/examples/cli/CMakeLists.txt for the real target name and update this check")
endif()
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.notedio)
endif()
set_target_properties(notedio PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
include(GNUInstallDirs)
install(TARGETS notedio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(notedio)
endif()