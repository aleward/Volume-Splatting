cmake_minimum_required(VERSION 3.10)
project(volume-splatting)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable C++17 for host code
set(CMAKE_CXX_STANDARD 17)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

########################################
# CUDA Setup
########################################
find_package(CUDA 10 REQUIRED)
include(${CMAKE_MODULE_PATH}/CUDAComputesList.cmake)

list(APPEND CUDA_NVCC_FLAGS ${CUDA_GENERATE_CODE})
list(APPEND CUDA_NVCC_FLAGS_DEBUG "-g -G")
set(CUDA_VERBOSE_BUILD ON)

if(WIN32)
    # Set up include and lib paths
    set(CUDA_HOST_COMPILER ${CMAKE_CXX_COMPILER} CACHE FILEPATH "Host side compiler used by NVCC" FORCE)
endif(WIN32)
########################################

find_package(OpenGL REQUIRED)

if(UNIX)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    set(LIBRARIES glfw ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY})
else(UNIX)
    set(EXTERNAL "external")

    set(GLFW_ROOT_DIR ${EXTERNAL})
    set(GLFW_USE_STATIC_LIBS ON)
    find_package(GLFW REQUIRED)

    set(GLEW_ROOT_DIR ${EXTERNAL})
    set(GLEW_USE_STATIC_LIBS ON)
    find_package(GLEW REQUIRED)

    add_definitions(${GLEW_DEFINITIONS})
    include_directories(${GLEW_INCLUDE_DIR} ${GLFW_INCLUDE_DIR})
    set(LIBRARIES ${GLEW_LIBRARY} ${GLFW_LIBRARY} ${OPENGL_LIBRARY})
endif(UNIX)

set(GLM_ROOT_DIR "external")
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

set(ITK_DIR "src/ITK/lib/cmake/ITK-5.1")
find_package(ITK REQUIRED)
include_directories(${ITK_USE_FILE})

set(ITKDEBUG_DIR "src/ITK/lib-debug/cmake/ITK-5.1")
find_package(ITKDEBUG REQUIRED)
include_directories(${ITK_DEBUG_USE_FILE})

set(headers
    src/cudaMat4.hpp
    src/glslUtility.hpp
    src/kernel.h
    src/main.hpp
    src/utilityCore.hpp
    src/GUI.hpp
    src/CameraControls.hpp
	src/ITKReader.hpp
    )

set(sources
    src/glslUtility.cpp
    src/kernel.cu
    src/main.cpp
    src/utilityCore.cpp
    src/GUI.cpp
    src/CameraControls.cpp
	src/ITKReader.cpp
    )

set(shadersGL
    shaders/splat.frag.glsl
    shaders/splat.geom.glsl
    shaders/splat.vert.glsl
    )

set(guiLib
    src/ImGuiFileDialog/CustomImGuiFileDialogConfig.h
    src/ImGuiFileDialog/CustomFont.cpp
    src/ImGuiFileDialog/CustomFont.h
    src/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.cpp
    src/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h
    src/ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialogConfig.h
    src/ImGuiFileDialog/3rdparty/dirent.h
    )

set(imgui
    src/imgui/imconfig.h
    src/imgui/imgui.cpp
    src/imgui/imgui.h
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/backends/imgui_impl_glfw.cpp
    src/imgui/backends/imgui_impl_glfw.h
    src/imgui/backends/imgui_impl_opengl3.cpp
    src/imgui/backends/imgui_impl_opengl3.h
    src/imgui/imgui_internal.h
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imstb_rectpack.h
    src/imgui/imstb_textedit.h
    src/imgui/imstb_truetype.h
    )

list(SORT headers)
list(SORT sources)
list(SORT shadersGL)
list(SORT guiLib)
list(SORT imgui)

source_group(Headers FILES ${headers})
source_group(Sources FILES ${sources})
source_group(Shaders FILES ${shadersGL})
source_group(GuiLib FILES ${guiLib})
source_group(ImGui FILES ${imgui})


cuda_add_executable(${CMAKE_PROJECT_NAME} ${sources} ${headers} ${shadersGL} ${guiLib} ${imgui})
target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} optimized ${ITK_LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} debug ${ITK_DEBUG_LIBRARIES})

#add_custom_command(
#    TARGET ${CMAKE_PROJECT_NAME}
#    PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/shaders
#        ${CMAKE_BINARY_DIR}/shaders
#    )
