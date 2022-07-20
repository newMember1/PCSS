cmake_minimum_required(VERSION 3.10)

set(projectName "PCSS")
set(ROOT_DIR ${CMAKE_SOURCE_DIR})

set(IMGUI_DIR "/home/zdxiao/Desktop/imgui")

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${ROOT_DIR}/include)
link_directories(${ROOT_DIR}/lib)

file(GLOB IMGUI ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
file(GLOB IMGUI_BACKENDS ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

add_executable(${projectName} ${ROOT_DIR}/src/main.cpp ${ROOT_DIR}/src/scene.cpp ${ROOT_DIR}/src/glad.c ${ROOT_DIR}/src/objloader.cpp ${ROOT_DIR}/src/gui.cpp ${IMGUI} ${IMGUI_BACKENDS})

target_link_libraries(${projectName} ${ROOT_DIR}/lib/libglfw3.a dl m GL X11 pthread)