project(pietcreator)
cmake_minimum_required(VERSION 2.6)

set( CMAKE_BUILD_TYPE Debug)
set( DEBUG_BUILD_TYPE ON )

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

add_subdirectory( npiet )

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ./npiet .)

set( pietcreator_SRCS
    main.cpp
    MainWindow.cpp
    ImageModel.cpp
    PixelDelegate.cpp
    KColorCells.cpp
    KColorMimeData.cpp
    KColorPatch.cpp
    ViewMonitor.cpp
    Command.cpp
    CommandsModel.cpp
    CommandDelegate.cpp
    ResizeDialog.cpp
    RunController.cpp
    NPietObserver.cpp
    CommandWidget.cpp
    DebugWidget.cpp
    CommandImpl.cpp
    FlowCompass.cpp
    UndoCommands.cpp
    UndoHandler.cpp )

set( CMAKE_AUTOMOC ON )

set( pietcreator_RCS
    fallback.qrc
    pietcreator.qrc )

qt5_wrap_cpp( pietcreator_SRCS )

QT5_ADD_RESOURCES( pietcreator_SRCS ${pietcreator_RCS} )

qt5_wrap_ui( pietcreator_SRCS MainWindow.ui DebugWidget.ui )


IF(WIN32) 
add_executable(pietcreator ${pietcreator_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/pietcreator.rc)
else()
add_executable(pietcreator ${pietcreator_SRCS})
endif(WIN32)

target_link_libraries( pietcreator
		       Qt5::Gui
		       Qt5::Core
		       Qt5::Widgets
                       npiet
                         )
