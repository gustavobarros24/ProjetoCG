cmake_minimum_required(VERSION 3.5)
project(CGProject1)

set(CMAKE_CXX_STANDARD 17)

# Ensure the standard is supported
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions
set(CMAKE_CXX_EXTENSIONS OFF)       

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/tinyXML2)

# List of header files
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

# Create a library for tinyxml2
add_library(tinyxml2 STATIC ${CMAKE_SOURCE_DIR}/include/tinyXML2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC ${CMAKE_SOURCE_DIR}/include/tinyXML2)

# Generator executable
add_executable(generator generator/generator.cpp ${HEADER_FILES})
target_include_directories(generator PRIVATE generator)
target_link_libraries(generator ${OPENGL_LIBRARIES})

# Engine executable
add_executable(engine engine/engine.cpp ${HEADER_FILES})
target_include_directories(engine PRIVATE engine)
target_link_libraries(engine tinyxml2 ${OPENGL_LIBRARIES})

# Organize files in Visual Studio
source_group("Header Files" FILES ${HEADER_FILES})

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

if (NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif()

if (WIN32)
    set(TOOLKITS_FOLDER "" CACHE PATH "Path to Toolkits folder")
    include_directories(${TOOLKITS_FOLDER}/glut)
    
    target_link_libraries(generator ${TOOLKITS_FOLDER}/glut/glut32.lib)
    target_link_libraries(engine ${TOOLKITS_FOLDER}/glut/glut32.lib)

    if (EXISTS "${TOOLKITS_FOLDER}/glut/glut32.dll")
        file(COPY ${TOOLKITS_FOLDER}/glut/glut32.dll DESTINATION ${CMAKE_BINARY_DIR})
    endif()

    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
else()
    find_package(GLUT REQUIRED)
    include_directories(${GLUT_INCLUDE_DIR})
    link_directories(${GLUT_LIBRARY_DIRS})
    add_definitions(${GLUT_DEFINITIONS})

    target_link_libraries(generator ${GLUT_LIBRARY})
    target_link_libraries(engine ${GLUT_LIBRARY})

    if (NOT GLUT_FOUND)
        message(ERROR ": GLUT not found!")
    endif()
endif()
