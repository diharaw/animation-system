cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(ASM_HEADERS ${PROJECT_SOURCE_DIR}/src/skeletal_mesh.h 
                ${PROJECT_SOURCE_DIR}/src/animation.h
                ${PROJECT_SOURCE_DIR}/src/skeleton.h
                ${PROJECT_SOURCE_DIR}/src/anim_blend.h
                ${PROJECT_SOURCE_DIR}/src/anim_local_transform.h
                ${PROJECT_SOURCE_DIR}/src/anim_global_transform.h
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
                ${PROJECT_SOURCE_DIR}/src/anim_local_transform.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_global_transform.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_fabrik_ik.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_lookat_ik.cpp
                ${PROJECT_SOURCE_DIR}/src/blendspace_1d.cpp
                ${PROJECT_SOURCE_DIR}/src/blendspace_2d.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_offset.cpp
                ${PROJECT_SOURCE_DIR}/src/anim_sample.cpp)

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(APPLE)
    add_executable(AnimationStateMachine MACOSX_BUNDLE ${ASM_HEADERS} ${ASM_SOURCES})
    set(MACOSX_BUNDLE_BUNDLE_NAME "com.dihara.asm") 
else()
    add_executable(AnimationStateMachine ${ASM_HEADERS} ${ASM_SOURCES}) 
endif()

target_link_libraries(AnimationStateMachine dwSampleFramework)

if (EMSCRIPTEN)
    set_target_properties(AnimationStateMachine PROPERTIES LINK_FLAGS "--embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/Rifle_Walk_Fwd.fbx@mesh/Rifle/Rifle_Walk_Fwd.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/Rifle_Run_Fwd.fbx@mesh/Rifle/Rifle_Run_Fwd.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/Rifle_Sprint_Fwd.fbx@mesh/Rifle/Rifle_Sprint_Fwd.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Fwd.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Fwd.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Left_Up.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Left_Up.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Up.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Up.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Right_Up.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Right_Up.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Left.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Left.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Right.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Right.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Left_Down.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Left_Down.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Down.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Down.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Right_Down.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Right_Down.fbx --embed-file ${PROJECT_SOURCE_DIR}/mesh/Rifle/AimOffsets/Rifle_Aim_Right_Down.fbx@mesh/Rifle/AimOffsets/Rifle_Aim_Right_Down.fbx --embed-file ${PROJECT_SOURCE_DIR}/shader/vs.glsl@shader/vs.glsl --embed-file ${PROJECT_SOURCE_DIR}/shader/fs.glsl@shader/fs.glsl --embed-file ${PROJECT_SOURCE_DIR}/shader/skinning_vs.glsl@shader/skinning_vs.glsl --embed-file ${PROJECT_SOURCE_DIR}/shader/skinning_fs.glsl@shader/skinning_fs.glsl --embed-file ${PROJECT_SOURCE_DIR}/shader/bone_vs.glsl@shader/bone_vs.glsl --embed-file ${PROJECT_SOURCE_DIR}/shader/bone_fs.glsl@shader/bone_fs.glsl -O3 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_GLFW=3 -s USE_WEBGL2=1")
endif()

if (APPLE)
    add_custom_command(TARGET AnimationStateMachine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shader $<TARGET_FILE_DIR:AnimationStateMachine>/AnimationStateMachine.app/Contents/Resources/shader)
else()
    add_custom_command(TARGET AnimationStateMachine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shader $<TARGET_FILE_DIR:AnimationStateMachine>/shader)
endif()

set_property(TARGET AnimationStateMachine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")