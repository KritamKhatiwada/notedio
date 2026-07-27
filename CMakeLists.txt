cmake_minimum_required(VERSION 3.16)

project(notedio VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia Sql)

set(PROJECT_SOURCES
    main.cpp
    core/AppConfig.h
    core/AppConfig.cpp
    core/Note.h
    core/Note.cpp
    core/NoteStore.h
    core/NoteStore.cpp
    core/AudioChunkRecorder.h
    core/AudioChunkRecorder.cpp
    core/TranscriptionEngine.h
    core/TranscriptionEngine.cpp
    core/SummarizationEngine.h
    core/SummarizationEngine.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/NoteListItemWidget.h
    ui/NoteListItemWidget.cpp
    ui/NoteEditorWidget.h
    ui/NoteEditorWidget.cpp
    ui/RecordingWindow.h
    ui/RecordingWindow.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(notedio
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(notedio SHARED ${PROJECT_SOURCES})
    else()
        add_executable(notedio ${PROJECT_SOURCES})
    endif()
endif()

target_link_libraries(notedio PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Sql)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.notedio.app)
endif()

set_target_properties(notedio PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

include(GNUInstallDirs)
install(TARGETS notedio
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(notedio)
endif()
