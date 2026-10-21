cmake_minimum_required(VERSION 3.1...3.29)

project(brdf_demo)

include(FetchContent)

FetchContent_Declare(glfw GIT_REPOSITORY git@github.com:glfw/glfw.git)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm GIT_REPOSITORY git@github.com:g-truc/glm.git)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(imgui GIT_REPOSITORY git@github.com:ocornut/imgui.git)
FetchContent_MakeAvailable(imgui)


set(SOURCES
    src/main.cpp
    src/debug.cpp
    src/util.cpp
    src/shader.cpp
    src/camera.cpp
    src/app.cpp
    src/app_base.cpp

    deps/glad/src/glad.c

    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


add_executable(${PROJECT_NAME} ${SOURCES})


# Define GLFW_INCLUDE_NONE so no problems #including glfw and glad
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

# Set C++17 as standard
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} glfw)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ./deps/glad/include
    ${glfw_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)