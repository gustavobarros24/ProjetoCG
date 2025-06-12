cmake_minimum_required(VERSION 3.10)
project(CGProject3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)       
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# GLM setup
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# pugiXML setup
include(FetchContent)
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.13
)
FetchContent_MakeAvailable(pugixml)

# Engine headers
file(GLOB ENGINE_HEADER_FILES "${CMAKE_SOURCE_DIR}/include/engine/*.h")
# Generator headers
file(GLOB GENERATOR_HEADER_FILES "${CMAKE_SOURCE_DIR}/include/generator/*.h")

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif()

if (WIN32)
    set(TOOLKITS_FOLDER "" CACHE PATH "Path to Toolkits folder")
    
    # Check for required libraries
    if (NOT EXISTS "${TOOLKITS_FOLDER}/glut/GL/glut.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glut/glut32.lib")
        message(ERROR ": GLUT not found")
    endif()
    
    if (NOT EXISTS "${TOOLKITS_FOLDER}/glew/GL/glew.h" OR NOT EXISTS "${TOOLKITS_FOLDER}/glew/glew32.lib")
        message(ERROR ": GLEW not found")
    endif()
    
    include_directories(${TOOLKITS_FOLDER}/glut ${TOOLKITS_FOLDER}/glew)
    
    # Copy DLLs to build directory
    if (EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll" AND EXISTS "${TOOLKITS_FOLDER}/glew/glew32.dll")
        file(COPY ${TOOLKITS_FOLDER}/glut/glut32.dll DESTINATION ${CMAKE_BINARY_DIR})
        file(COPY ${TOOLKITS_FOLDER}/glew/glew32.dll DESTINATION ${CMAKE_BINARY_DIR})
    endif()
    
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
else()
    find_package(GLUT REQUIRED)
    include_directories(${GLUT_INCLUDE_DIR})
    link_directories(${GLUT_LIBRARY_DIRS})
    add_definitions(${GLUT_DEFINITIONS})

    if (NOT APPLE)
        find_package(GLEW REQUIRED)
        include_directories(${GLEW_INCLUDE_DIRS})
        link_libraries(${GLEW_LIBRARIES})
    endif()

    if(NOT GLUT_FOUND)
        message(ERROR ": GLUT not found!")
    endif()
endif()

# Generator executable
add_executable(generator generator/generator.cpp ${GENERATOR_HEADER_FILES})
target_include_directories(generator 
    PRIVATE 
        generator
        ${CMAKE_SOURCE_DIR}/include/generator
)
if (WIN32)
    target_link_libraries(generator ${OPENGL_LIBRARIES} glm::glm
        ${TOOLKITS_FOLDER}/glut/glut32.lib
        ${TOOLKITS_FOLDER}/glew/glew32.lib)
else()
    if (NOT APPLE)
        target_link_libraries(generator ${OPENGL_LIBRARIES} glm::glm ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})
    else()
        target_link_libraries(generator ${OPENGL_LIBRARIES} glm::glm ${GLUT_LIBRARIES})
    endif()
endif()

# Engine executable
add_executable(engine engine/engine.cpp ${ENGINE_HEADER_FILES})
target_include_directories(engine 
    PRIVATE 
        engine
        ${CMAKE_SOURCE_DIR}/include/engine
)
target_link_libraries(engine ${OPENGL_LIBRARIES} glm::glm pugixml)
if (WIN32)
    target_link_libraries(engine
        ${TOOLKITS_FOLDER}/glut/glut32.lib
        ${TOOLKITS_FOLDER}/glew/glew32.lib)
else()
    if (NOT APPLE)
        target_link_libraries(engine ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})
    else()
        target_link_libraries(engine ${GLUT_LIBRARIES})
    endif()
endif()

# Organize files in Visual Studio
source_group("Engine Headers" FILES ${ENGINE_HEADER_FILES})
source_group("Generator Headers" FILES ${GENERATOR_HEADER_FILES})