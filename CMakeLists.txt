cmake_minimum_required(VERSION 3.29)
project(beta2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

add_executable(beta2 src/main.cpp
        src/models/movie.cpp
        includes/models/movie.h
        src/models/MovieCollection.cpp
        includes/models/MovieCollection.h
        src/exceptions/MovieException.cpp
        includes/exceptions/MovieException.h
        src/exceptions/FileNotFoundException.cpp
        includes/exceptions/FileNotFoundException.h
        src/exceptions/InvalidMovieDataException.cpp
        includes/exceptions/InvalidMovieDataException.h
        src/exceptions/MovieNotFoundException.cpp
        includes/exceptions/MovieNotFoundException.h
        src/exceptions/DuplicateFavoriteException.cpp
        includes/exceptions/DuplicateFavoriteException.h
        src/exceptions/DuplicateMovieException.cpp
        includes/exceptions/DuplicateMovieException.h
        src/exceptions/InvalidInputException.cpp
        includes/exceptions/InvalidInputException.h
        src/exceptions/CollectionNotFoundException.cpp
        includes/exceptions/CollectionNotFoundException.h
        src/exceptions/DuplicateCollectionException.cpp
        includes/exceptions/DuplicateCollectionException.h
        src/exceptions/EmptyCollectionException.cpp
        includes/exceptions/EmptyCollectionException.h
        includes/exceptions/exceptions.h
        src/repositories/MovieRepository.cpp
        includes/repositories/MovieRepository.h
        src/repositories/FavoriteRepository.cpp
        includes/repositories/FavoriteRepository.h
        src/repositories/CollectionRepository.cpp
        includes/repositories/CollectionRepository.h
        src/services/MovieService.cpp
        includes/services/MovieService.h
        src/services/FavoriteService.cpp
        includes/services/FavoriteService.h
        src/services/CollectionService.cpp
        includes/services/CollectionService.h
        src/parsers/MovieJsonParser.cpp
        includes/parsers/MovieJsonParser.h
        src/parsers/MovieParser.cpp
        includes/parsers/MovieParser.h
        src/managers/MovieManager.cpp
        includes/managers/MovieManager.h
        src/managers/PosterManager.cpp
        includes/managers/PosterManager.h
        src/ui/MainWindow.cpp
        includes/ui/MainWindow.h
        src/api/KinopoiskAPIClient.cpp
        includes/api/KinopoiskAPIClient.h
        src/ui/MovieCardFactory.cpp
        includes/ui/MovieCardFactory.h)

# Add includes directory to include path
target_include_directories(beta2 PRIVATE ${CMAKE_SOURCE_DIR}/includes)

target_link_libraries(beta2
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
)

if (EXISTS "${CMAKE_SOURCE_DIR}/posters")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/posters"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/posters"
            COMMENT "Copying posters directory"
    )
endif()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets Network)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
    
    # Копируем OpenSSL DLL для поддержки HTTPS
    if (EXISTS "${QT_INSTALL_PATH}/bin/libcrypto-3-x64.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/libcrypto-3-x64.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()
    
    if (EXISTS "${QT_INSTALL_PATH}/bin/libssl-3-x64.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/libssl-3-x64.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()
    
    # Альтернативные имена для старых версий OpenSSL
    if (EXISTS "${QT_INSTALL_PATH}/bin/libcrypto-1_1-x64.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/libcrypto-1_1-x64.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()
    
    if (EXISTS "${QT_INSTALL_PATH}/bin/libssl-1_1-x64.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/libssl-1_1-x64.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif()
    
           # Копируем SSL плагины
           if (EXISTS "${QT_INSTALL_PATH}/plugins/tls")
               add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory
                       "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls")
               add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${QT_INSTALL_PATH}/plugins/tls"
                       "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/tls")
           endif()
           
           # Копируем плагины для форматов изображений (JPEG, PNG и т.д.)
           if (EXISTS "${QT_INSTALL_PATH}/plugins/imageformats")
               add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory
                       "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats")
               add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${QT_INSTALL_PATH}/plugins/imageformats"
                       "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats")
           endif()
       endif ()
