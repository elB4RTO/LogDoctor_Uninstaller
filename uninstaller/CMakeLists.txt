cmake_minimum_required(VERSION 3.5)

project(LogDoctorUninstaller VERSION 2.00 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W2)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

if(${QT_VERSION_MAJOR} LESS 6)
    message("Error: Qt6 or greater required")
    return()
endif()


set(TS_FILES
    translations/LogDoctorUninstaller_en.ts
    translations/LogDoctorUninstaller_es.ts
    translations/LogDoctorUninstaller_fr.ts
    translations/LogDoctorUninstaller_it.ts
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    modules/dialogs/dialogbool.ui
    modules/dialogs/dialogbool.h
    modules/dialogs/dialogbool.cpp

    resources/resources.qrc
    ${TS_FILES}
)

qt_add_executable(LogDoctorUninstaller
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic -lgcc -lstdc++ -lwinpthread")
    target_link_libraries(LogDoctorUninstaller PRIVATE
        -static Qt${QT_VERSION_MAJOR}::Widgets
    )
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
    target_link_libraries(LogDoctorUninstaller PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
    )
endif()


if(WIN32)
    set_target_properties(LogDoctorUninstaller PROPERTIES
        WIN32_EXECUTABLE TRUE)
endif()

qt_finalize_executable(LogDoctorUninstaller)
