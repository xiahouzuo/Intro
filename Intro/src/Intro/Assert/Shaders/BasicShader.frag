#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1; // 与Mesh.cpp中的设置保持一致

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // 如果纹理是透明的，显示为红色以便调试
    if(texColor.a < 0.1)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        FragColor = texColor;
}