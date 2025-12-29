cmake_minimum_required(VERSION 3.15)

project(Volgagian VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Dependencies ---

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# --- Source Management ---
# Note: We only glob sources from 'src'. External libs are header-only or handled separately.
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Ensure the 'external' folder exists
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/external")

# --- Dependency: GLM (Math) ---
# Checked in 'external/glm' instead of 'src/glm'
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glm")
    message(STATUS "Downloading GLM...")
    file(DOWNLOAD
        "https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip"
        "${CMAKE_BINARY_DIR}/glm.zip"
    )
    # Extract
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/glm.zip WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # Move to 'external'
    file(RENAME "${CMAKE_BINARY_DIR}/glm-0.9.9.8/glm" "${CMAKE_SOURCE_DIR}/external/glm")
endif()

# --- Dependency: tinygltf & Friends ---
# Checked and downloaded to 'external/'
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/tiny_gltf.h")
    message(STATUS "Downloading tinygltf headers...")
    file(DOWNLOAD "https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h" "${CMAKE_SOURCE_DIR}/external/tiny_gltf.h")
    file(DOWNLOAD "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp" "${CMAKE_SOURCE_DIR}/external/json.hpp")
    file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "${CMAKE_SOURCE_DIR}/external/stb_image.h")
    file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "${CMAKE_SOURCE_DIR}/external/stb_image_write.h")
endif()

# --- Target Definition ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include Directories ---
# Added 'external' so #include <glm/...> and #include "tiny_gltf.h" still work
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    external
    ${GLEW_INCLUDE_DIRS}
)

# --- Linking ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    ${GLEW_LIBRARIES}
    OpenGL::GL
)

# --- Post-Build Steps (Assets & DLLs) ---

# 1. Copy 'data' folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data
    COMMENT "Copying data assets..."
)

# 2. Copy GLEW DLL (Windows Only)
if(WIN32)
    get_filename_component(GLEW_LIB_DIR "${GLEW_LIBRARIES}" DIRECTORY)
    find_file(GLEW_DLL "glew32.dll"
        HINTS "${GLEW_LIB_DIR}/../bin" "${GLEW_LIB_DIR}"
        NO_DEFAULT_PATH
    )

    if(GLEW_DLL)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_DLL}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
            COMMENT "Copying GLEW DLL to binary directory..."
        )
    else()
        message(WARNING "Could not auto-locate glew32.dll. You may need to copy it manually to build/bin/.")
    endif()
endif()
