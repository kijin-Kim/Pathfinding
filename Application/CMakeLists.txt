cmake_minimum_required(VERSION 4.0)
project(Application LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE APPLICATION_SOURCE_FILES src/*.cpp src/*.h)
add_executable(Application ${APPLICATION_SOURCE_FILES}
        src/Layers/PathfindingLayer.cpp
        src/Layers/PathfindingLayer.h
        src/Layers/ImGuiLayer.cpp
        src/Layers/ImGuiLayer.h
        src/Layers/LayerCommon.h
        src/Layers/TilemapEditor/TileSetData.cpp
        src/Layers/TilemapEditor/TileSetData.h
        src/Layers/TilemapEditor/MapData.cpp
        src/Layers/TilemapEditor/MapData.h
        src/Layers/TilemapEditor/DebugUI.cpp
        src/Layers/TilemapEditor/DebugUI.h)

target_link_libraries(Application PRIVATE CommonCore)
target_include_directories(Application PRIVATE src)

set(CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/config/")

set(APP_ASSET_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(Application PRIVATE CONFIG_PATH="${CONFIG_PATH}" APP_ASSET_PATH="${APP_ASSET_PATH}" MAPS_PATH="${MAPS_PATH}" NOMINMAX)