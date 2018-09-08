layout (location = 0) in vec3 VS_IN_Position;
layout (location = 1) in vec2 VS_IN_Texcoord;
layout (location = 2) in vec3 VS_IN_Normal;
layout (location = 3) in vec3 VS_IN_Tangent;
layout (location = 4) in ivec4 VS_IN_BoneIDs;
layout (location = 5) in vec4 VS_IN_Weights;

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

void main()
{
	//mat4 bone_transform = bones[VS_IN_BoneIDs[0]] * VS_IN_Weights[0];
    //bone_transform     += bones[VS_IN_BoneIDs[1]] * VS_IN_Weights[1];
   // bone_transform     += bones[VS_IN_BoneIDs[2]] * VS_IN_Weights[2];
    //bone_transform     += bones[VS_IN_BoneIDs[3]] * VS_IN_Weights[3];

    vec4 position = vec4(VS_IN_Position, 1.0f);

    vec4 world_pos = model * position;
    PS_IN_FragPos = world_pos.xyz;

	mat3 model_bone_bat = mat3(model);

	PS_IN_Normal = normalize(model_bone_bat * VS_IN_Normal);

	gl_Position = projection * view * world_pos;
}
