cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(ASM_HEADERS ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.h 
                ${PROJECT_SOURCE_DIR}/src/animation.h)

set(ASM_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp
                ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.cpp
                ${PROJECT_SOURCE_DIR}/src/animation.cpp)

if(APPLE)
    add_executable(AnimationStateMachine MACOSX_BUNDLE ${ASM_HEADERS} ${ASM_SOURCES})
    set(MACOSX_BUNDLE_BUNDLE_NAME "com.dihara.asm") 
else()
    add_executable(AnimationStateMachine ${ASM_HEADERS} ${ASM_SOURCES}) 
endif()

target_link_libraries(AnimationStateMachine dwSampleFramework)