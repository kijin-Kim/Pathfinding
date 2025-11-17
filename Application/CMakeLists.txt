cmake_minimum_required(VERSION 4.0)
project(Application LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE APPLICATION_SOURCE_FILES src/*.cpp src/*.h)
add_executable(Application ${APPLICATION_SOURCE_FILES})

target_link_libraries(Application PRIVATE CommonCore)
target_include_directories(Application PRIVATE src)
target_compile_definitions(Application PRIVATE NOMINMAX)

add_custom_command(TARGET Application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/CommonCore/src/Renderer/Assets"
        "$<TARGET_FILE_DIR:Application>/Assets"
)

if (MINGW)
    target_link_options(Application PRIVATE
            -static-libgcc
            -static-libstdc++
            -static -lpthread
    )
endif ()

install(TARGETS Application
        RUNTIME DESTINATION bin
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/CommonCore/src/Renderer/Assets"
        DESTINATION bin
)

install(FILES "$<TARGET_FILE_DIR:Application>/imgui.ini"
        DESTINATION bin
)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "Pathfinding")
set(CPACK_PACKAGE_VERSION "1.0.0")
include(CPack)

