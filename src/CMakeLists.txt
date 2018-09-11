cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(ASM_HEADERS ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.h 
                ${PROJECT_SOURCE_DIR}/src/animation.h
                ${PROJECT_SOURCE_DIR}/src/skeleton.h
                ${PROJECT_SOURCE_DIR}/src/anim_blend.h
                ${PROJECT_SOURCE_DIR}/src/anim_bone_to_local.h
                ${PROJECT_SOURCE_DIR}/src/anim_override.h
                ${PROJECT_SOURCE_DIR}/src/anim_sample.h)

set(ASM_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp
                ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.cpp
                ${PROJECT_SOURCE_DIR}/src/animation.cpp
                ${PROJECT_SOURCE_DIR}/src/skeleton.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_blend.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_bone_to_local.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_override.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_sample.cpp)

if(APPLE)
    add_executable(AnimationStateMachine MACOSX_BUNDLE ${ASM_HEADERS} ${ASM_SOURCES})
    set(MACOSX_BUNDLE_BUNDLE_NAME "com.dihara.asm") 
else()
    add_executable(AnimationStateMachine ${ASM_HEADERS} ${ASM_SOURCES}) 
endif()

target_link_libraries(AnimationStateMachine dwSampleFramework)


if (APPLE)
    add_custom_command(TARGET AnimationStateMachine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shader $<TARGET_FILE_DIR:AnimationStateMachine>/AnimationStateMachine.app/Contents/Resources/shader)
else()
    add_custom_command(TARGET AnimationStateMachine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shader $<TARGET_FILE_DIR:AnimationStateMachine>/shader)
endif()
