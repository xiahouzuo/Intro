#version 330 core

layout (location = 0) in vec3 aPos;      // ��Mesh��Vertex.Position��Ӧ��location=0��
layout (location = 1) in vec3 aNormal;   // ��ʱ���ã�����
layout (location = 2) in vec2 aTexCoords;// ��ʱ���ã�����

uniform mat4 model;      // ģ�;�����������ı任��
uniform mat4 view;       // ��ͼ����������ı任��
uniform mat4 projection; // ͶӰ����͸��/������

void main() {
    // ���ģ��������� = ͶӰ���� * ��ͼ���� * ģ�;��� * �ֲ�����
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}