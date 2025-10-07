#version 410 core

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_Proj;
	vec4 u_ViewPos;
	float u_Time;
	vec3 _pad1;
};