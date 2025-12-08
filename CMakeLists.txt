cmake_minimum_required(VERSION 3.16)

# Sets project name
project(GEARUP_LETSFUCKIINGGOO LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define resource directories
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/scenefiles")
set(TEXTURE_DIR "${RESOURCE_DIR}/maps")

# Check if resource directory exists
if(NOT EXISTS ${RESOURCE_DIR})
    message(WARNING "Resource directory 'scenefiles' not found at: ${RESOURCE_DIR}")
else()
    message(STATUS "Found resource directory: ${RESOURCE_DIR}")
endif()

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/realtime/physics.cpp
    src/realtime/rendering.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp
    src/utils/shapefactory.cpp
    src/utils/camera.cpp

    src/mainwindow.h
    src/realtime.h
    src/realtime/physics.h
    src/realtime/rendering.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shapefactory.h
    src/utils/camera.h
    src/utils/transforms.h
    src/utils/shaderloader.h

    src/utils/debug.h

    src/utils/aspectratiowidget/aspectratiowidget.hpp

    src/shapes/Cube.cpp
    src/shapes/Sphere.cpp
    src/shapes/Cone.cpp
    src/shapes/Cylinder.cpp
    src/shapes/Shape.h
    src/blocks/Block.cpp
    src/blocks/Block.h

    src/map/mapbuilderwidget.cpp
    src/map/mapbuilderwidget.h
    src/map/mapbuilder.cpp
    src/map/mapbuilder.h
    src/map/Map.cpp
    src/map/Map.h
    src/map/Chunk.cpp
    src/map/Chunk.h
    src/map/mapproperties.cpp
    src/map/mapproperties.h
)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c
)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/default.frag
        resources/shaders/default.vert
        resources/shaders/debug.frag
        resources/shaders/debug.vert
        resources/shaders/phong.frag
        resources/shaders/phong.vert
)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_BINARY_DIR}/scenefiles/maps"
    COMMENT "Creating resource directories in build folder"
)

# Copy texture files from source to build directory
file(GLOB_RECURSE TEXTURE_FILES
    "${TEXTURE_DIR}/*.png"
    "${TEXTURE_DIR}/*.jpg"
    "${TEXTURE_DIR}/*.jpeg"
    "${TEXTURE_DIR}/*.bmp"
    "${TEXTURE_DIR}/*.tga"
)

foreach(texture_file ${TEXTURE_FILES})
    # Get just the filename
    get_filename_component(FILENAME ${texture_file} NAME)

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${texture_file}"
            "${CMAKE_BINARY_DIR}/scenefiles/maps/${FILENAME}"
        COMMENT "Copying texture: ${FILENAME}"
    )
endforeach()

# Copy any other scene files if they exist
if(EXISTS ${RESOURCE_DIR})
    file(GLOB_RECURSE SCENE_FILES
        "${RESOURCE_DIR}/*.xml"
        "${RESOURCE_DIR}/*.json"
        "${RESOURCE_DIR}/*.txt"
        "${RESOURCE_DIR}/*.scene"
    )

    foreach(scene_file ${SCENE_FILES})
        # Get relative path from resource directory
        file(RELATIVE_PATH REL_PATH ${RESOURCE_DIR} ${scene_file})
        get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)

        if(REL_DIR)
            add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${CMAKE_BINARY_DIR}/scenefiles/${REL_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${scene_file}"
                    "${CMAKE_BINARY_DIR}/scenefiles/${REL_PATH}"
                COMMENT "Copying scene file: ${REL_PATH}"
            )
        else()
            add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${scene_file}"
                    "${CMAKE_BINARY_DIR}/scenefiles/"
                COMMENT "Copying scene file: ${scene_file}"
            )
        endif()
    endforeach()
endif()
