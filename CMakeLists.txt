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
    resources/shaders/block.frag
    resources/shaders/block.vert
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
