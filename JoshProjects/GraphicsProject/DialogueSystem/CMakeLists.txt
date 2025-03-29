cmake_minimum_required(VERSION 3.10)
project(DialogueSystem VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JAGEngine directory
set(JAGENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JAGEngine" CACHE PATH "Path to JAGEngine")

# Find required packages
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)

# Find nlohmann_json (optional - can be added as submodule if needed)
find_package(nlohmann_json CONFIG)
if(NOT nlohmann_json_FOUND)
    # If not found, add as a subdirectory (requires the repository to be cloned)
    message(STATUS "nlohmann_json not found, will try to use submodule")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/json/CMakeLists.txt")
        add_subdirectory(external/json)
    else()
        message(FATAL_ERROR "nlohmann_json not found and submodule not available. Please run: git submodule update --init --recursive")
    endif()
endif()

# Source files
set(SOURCES
    DialogueSystem.cpp
    DialogueEditor.cpp
    DialogueApp.cpp
)

set(HEADERS
    DialogueSystem.h
)

# Add the executable
add_executable(DialogueSystem ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(DialogueSystem PRIVATE
    ${JAGENGINE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(DialogueSystem PRIVATE
    JAGEngine
    ${OPENGL_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# Copy necessary files to build directory
add_custom_command(TARGET DialogueSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:DialogueSystem>/Audio/Dialogue
)

# Set output directory
set_target_properties(DialogueSystem PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Installation
install(TARGETS DialogueSystem DESTINATION bin)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Audio" DESTINATION bin)