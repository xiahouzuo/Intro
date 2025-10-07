#version 410 core

const int MAX_DIR_LIGHTS = 4;
const int MAX_POINT_LIGHTS = 4;
const int MAX_SPOT_LIGHTS = 4;

struct DirLight {
	vec4 direction; //xyz w
	vec4 color;     //rgb w
};

struct PointLight {
	vec4 Position;
	vec4 color;
};

struct SpotLight {
	vec4 position;
	vec4 direction;
	vec4 color;
	vec4 params;
};

layout(std140, binding = 1) uniform LightsUBO {
	int numDir;
	int numPoint;
	int numSpot;
	vec2 _pad0;
	DirLight dirLights[MAX_DIR_LIGHTS];
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};