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

# Optional: Qt Multimedia for sound effects
find_package(Qt6 QUIET COMPONENTS Multimedia)
if(Qt6Multimedia_FOUND)
    message(STATUS "Qt6 Multimedia found - sound effects enabled")
    add_definitions(-DQT_MULTIMEDIA_LIB)
else()
    message(STATUS "Qt6 Multimedia not found - sound effects disabled")
endif()

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/realtime/physics.cpp
    src/realtime/rendering.cpp
    src/realtime/gbuffer.cpp
    src/realtime/textures.cpp
    src/realtime/fog.cpp
    src/realtime/input.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp
    src/utils/shapefactory.cpp
    src/utils/camera.cpp
    src/utils/camerapath.cpp
    src/utils/camerapath.h
    src/utils/audiomanager.cpp
    src/utils/audiomanager.h

    src/mainwindow.h
    src/realtime.h
    src/realtime/physics.h
    src/realtime/rendering.h
    src/realtime/gbuffer.h
    src/realtime/textures.h
    src/realtime/fog.h
    src/realtime/input.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shapefactory.h
    src/utils/camera.h
    src/utils/transforms.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp

    src/shapes/Cube.cpp
    src/shapes/Sphere.cpp
    src/shapes/Cone.cpp
    src/shapes/Cylinder.cpp
    src/shapes/Shape.h
    src/blocks/Block.cpp
    src/blocks/Block.h
    src/blocks/CompletionCubePiece.cpp
    src/blocks/CompletionCubePiece.h
    src/blocks/TreePiece.cpp
    src/blocks/TreePiece.h

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
    src/map/terraintreegenerator.cpp
    src/map/terraintreegenerator.h
    src/map/CompletionCube.cpp
    src/map/CompletionCube.h

    src/lsystem/lsystemwidget.cpp
    src/lsystem/lsystemwidget.h
    src/lsystem/axialtree.h
    src/lsystem/treegenerator.cpp
    src/lsystem/treegenerator.h
    
    src/enemies/enemy.cpp
    src/enemies/enemy.h
    src/enemies/enemymanager.cpp
    src/enemies/enemymanager.h
    
    src/particlesystem/particlesystem.cpp
    src/particlesystem/particlesystem.h
    
    src/ui/ui.cpp
    src/ui/ui.h
)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c
    src/utils/debug.h)
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
        resources/shaders/gbuffer.frag
        resources/shaders/gbuffer.vert
        resources/shaders/motionblur.frag
        resources/shaders/motionblur.vert
        resources/shaders/depthviz.frag
        resources/shaders/depthviz.vert
        resources/shaders/gbufferviz.frag
        resources/shaders/gbufferviz.vert
        resources/shaders/post.frag
        resources/shaders/post.vert
        resources/shaders/postfilter.frag
        resources/shaders/postfilter.vert
        resources/shaders/blur.frag
        resources/shaders/blur.vert
        resources/shaders/bloomextract.frag
        resources/shaders/bloomcombine.frag
        resources/shaders/particles.frag
        resources/shaders/particles.vert
        resources/shaders/ui.frag
        resources/shaders/ui.vert
        resources/shaders/default.vert
        resources/shaders/default.frag
        resources/luts/neutral.png
        resources/luts/fancy.png
        resources/luts/warm.png
        resources/luts/blackandwhite.png
        resources/luts/cool.png
        resources/luts/green.png
        resources/luts/blue.png
        resources/luts/horror.png
        resources/textures/particles/dirtparticle1.png
        resources/textures/particles/dirtparticle2.png
        resources/textures/particles/dirtparticle3.png
        resources/textures/particles/wisp.png
        resources/textures/particles/dust_mountains.png
        resources/textures/particles/dust_forest.png
        resources/textures/particles/dust_grasslands.png
        resources/textures/wood_normal.png
        resources/textures/wood_bump.png
        resources/textures/dirt.png
        resources/textures/sand.jpg
        resources/soundeffects/flashlight_on.wav
        resources/soundeffects/flashlight_off.wav
        resources/soundeffects/enemyhit.wav
        resources/soundeffects/enemyhits/enemyhit.wav
        resources/soundeffects/enemyhits/enemyhit2.wav
        resources/soundeffects/enemyhits/enemyhit3.wav
        resources/soundeffects/enemyhits/enemyhit4.wav
        resources/soundeffects/enemyhits/enemyhit5.wav
        resources/soundeffects/enemyhits/enemyhit6.wav
        resources/soundeffects/enemyhits/enemyhit7.wav
        resources/soundeffects/ambientfinal.wav
        resources/soundeffects/random_ambient/ambient_bells.wav
        resources/soundeffects/random_ambient/ambient_chaos.wav
        resources/soundeffects/random_ambient/ambient_crackle.wav
        resources/soundeffects/random_ambient/ambient_crashwoosh.wav
        resources/soundeffects/random_ambient/ambient_idkwwhatocallthis.wav
        resources/soundeffects/random_ambient/ambient_steps.wav
        resources/soundeffects/random_ambient/ambient_vibrate.wav
        resources/soundeffects/random_ambient/ambient_woosh.wav
        resources/soundeffects/enemynoises/enemy_noise1.wav
        resources/soundeffects/enemynoises/enemy_noise2.wav
        resources/soundeffects/enemynoises/enemy_noise3.wav
        resources/soundeffects/enemynoises/enemy_noise4.wav
        resources/soundeffects/enemynoises/enemy_noise5.wav
        resources/soundeffects/enemydie/enemy_die.wav
        resources/soundeffects/enemydie/enemy_die1.wav
        resources/soundeffects/enemydie/enemy_die2.wav
        resources/soundeffects/enemydie/enemy_die3.wav
        resources/soundeffects/enemydie/enemy_die4.wav
        resources/soundeffects/footsteps/step1.wav
        resources/soundeffects/footsteps/step2.wav
        resources/soundeffects/footsteps/step3.wav
        resources/soundeffects/footsteps/step4.wav
        resources/soundeffects/cube_grab.wav
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
