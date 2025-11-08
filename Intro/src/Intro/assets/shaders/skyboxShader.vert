#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos;
    // 移除平移部分，只保留旋转
    mat4 viewWithoutTranslation = mat4(mat3(view));
    vec4 pos = projection * viewWithoutTranslation * vec4(aPos, 1.0);
    gl_Position = pos.xyww;  // 设置z=w确保深度为1.0
}