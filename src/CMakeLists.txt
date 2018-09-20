cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(ASM_HEADERS ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.h 
                ${PROJECT_SOURCE_DIR}/src/animation.h
                ${PROJECT_SOURCE_DIR}/src/skeleton.h
                ${PROJECT_SOURCE_DIR}/src/anim_blend.h
                ${PROJECT_SOURCE_DIR}/src/anim_local_to_global.h
                ${PROJECT_SOURCE_DIR}/src/anim_fabrik_ik.h
                ${PROJECT_SOURCE_DIR}/src/anim_lookat_ik.h
                ${PROJECT_SOURCE_DIR}/src/blendspace_1d.h
                ${PROJECT_SOURCE_DIR}/src/blendspace_2d.h
                ${PROJECT_SOURCE_DIR}/src/anim_offset.h
                ${PROJECT_SOURCE_DIR}/src/anim_sample.h)

set(ASM_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp
                ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.cpp
                ${PROJECT_SOURCE_DIR}/src/animation.cpp
                ${PROJECT_SOURCE_DIR}/src/skeleton.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_blend.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_local_to_global.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_fabrik_ik.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_lookat_ik.cpp
                ${PROJECT_SOURCE_DIR}/src/blendspace_1d.cpp
                ${PROJECT_SOURCE_DIR}/src/blendspace_2d.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_offset.cpp
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
