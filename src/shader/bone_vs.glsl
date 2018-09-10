layout (location = 0) in vec3 VS_IN_Position;
layout (location = 1) in vec3 VS_IN_Normal;

const int MAX_BONES = 128;

layout (std140) uniform u_GlobalUBO
{ 
    mat4 view;
    mat4 projection;
};

layout (std140) uniform u_ObjectUBO
{ 
    mat4 model;
};

layout (std140) uniform u_BoneUBO
{ 
    mat4 bones[MAX_BONES];
};

out vec3 PS_IN_FragPos;
out vec3 PS_IN_Normal;

mat4 get_world_matrix(mat4 joint) 
{
    joint = transpose(joint);
    // Rebuilds bone properties.
    // Bone length is set to zero to disable leaf rendering.
    float is_bone = joint[3].w;
    vec3 bone_dir = vec3(joint[0].w, joint[1].w, joint[2].w) * is_bone;
    float bone_len = length(bone_dir); 
    // Setup rendering world matrix.
    float dot = dot(joint[2].xyz, bone_dir);
    vec3 binormal = abs(dot) < .01 ? joint[2].xyz : joint[1].xyz;

    mat4 world_matrix;
    world_matrix[0] = vec4(bone_dir, 0.);
    world_matrix[1] = vec4(bone_len * normalize(cross(binormal, bone_dir)), 0.);
    world_matrix[2] = vec4(bone_len * normalize(cross(bone_dir, world_matrix[1].xyz)), 0.0);
    world_matrix[3] = vec4(joint[3].xyz, 1.);
    return world_matrix;
}

void main()
{
    mat4 model_mat = model * (bones[gl_InstanceID]);
    vec4 position = model_mat * vec4(VS_IN_Position, 1.0f);

    PS_IN_FragPos = position.xyz;

	mat3 model_bone_bat = mat3(model_mat);

	PS_IN_Normal = normalize(model_bone_bat * VS_IN_Normal);

	gl_Position = projection * view * position;
}
