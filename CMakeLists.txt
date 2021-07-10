cmake_minimum_required(VERSION 3.16)
project(opengl_core)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_executable(Application
Application/src/vendor/include/glm/detail/glm.cpp
Application/src/vendor/glad/glad.c
Application/src/vendor/include/stb_image/stb_image.cpp
Application/src/vendor/include/tinyobj/tiny_obj_loader.cc
Application/src/vendor/include/imgui/imgui.cpp
Application/src/vendor/include/imgui/imgui_draw.cpp
Application/src/vendor/include/imgui/imgui_impl_glfw.cpp
Application/src/vendor/include/imgui/imgui_impl_opengl3.cpp
Application/src/vendor/include/imgui/imgui_widgets.cpp
Application/src/application.cpp
Application/src/include/scene.h 
Application/src/include/material.h)

target_include_directories(Application PRIVATE Application/src/include)
target_include_directories(Application PRIVATE Application/src/vendor/include)



if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_link_directories(Application PRIVATE Application/libs/glfw/)

find_package(assimp)
if (assimp_FOUND)
target_link_libraries (Application PRIVATE  assimp::assimp)
message("Assimp found")
add_compile_definitions(USEASSIMP)
endif(assimp_FOUND)

target_link_libraries(Application PRIVATE glfw3.lib)
message("Linked to windows using MSVC")

else()

if (CMAKE_SYSTEM_NAME STREQUAL Linux)        
target_link_libraries(Application PRIVATE glfw pthread dl GL X11 assimp)       
add_compile_definitions(USEASSIMP)

message("Linked to Linux")

elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)   
find_package(assimp )
if (assimp_FOUND)
link_directories(${ASSIMP_LIBRARY_DIRS})
target_include_directories(Application PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_link_libraries (Application PRIVATE ${ASSIMP_LIBRARIES})
add_compile_definitions(USEASSIMP)
message("Assimp found")
endif(assimp_FOUND)
target_link_libraries(Application PRIVATE libglfw3)
message("Linked to windows")
endif ()
endif()
