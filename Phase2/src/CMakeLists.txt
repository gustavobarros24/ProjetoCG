cmake_minimum_required(VERSION 3.5)
project(CGProject2)

set(CMAKE_CXX_STANDARD 17)
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

# Engine headers
file(GLOB ENGINE_HEADER_FILES "${CMAKE_SOURCE_DIR}/include/engine/*.h")
# Generator headers
file(GLOB GENERATOR_HEADER_FILES "${CMAKE_SOURCE_DIR}/include/generator/*.h")

# Create a library for tinyxml2
add_library(tinyxml2 STATIC ${CMAKE_SOURCE_DIR}/include/tinyXML2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC ${CMAKE_SOURCE_DIR}/include/tinyXML2)

# Generator executable
add_executable(generator generator/generator.cpp ${GENERATOR_HEADER_FILES})
target_include_directories(generator 
    PRIVATE 
        generator
        ${CMAKE_SOURCE_DIR}/include/generator
)
target_link_libraries(generator ${OPENGL_LIBRARIES} glm::glm)

# Engine executable
add_executable(engine engine/engine.cpp ${ENGINE_HEADER_FILES})
target_include_directories(engine 
    PRIVATE 
        engine
        ${CMAKE_SOURCE_DIR}/include/engine
)
target_link_libraries(engine tinyxml2 ${OPENGL_LIBRARIES} glm::glm)

# Organize files in Visual Studio
source_group("Engine Headers" FILES ${ENGINE_HEADER_FILES})
source_group("Generator Headers" FILES ${GENERATOR_HEADER_FILES})

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

# Copy executables to solution root for easier access
foreach(TARGET_NAME engine generator)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:${TARGET_NAME}>"
            "${CMAKE_SOURCE_DIR}/../$<TARGET_FILE_NAME:${TARGET_NAME}>"
        COMMENT "Copying ${TARGET_NAME} executable to solution root"
    )
endforeach()