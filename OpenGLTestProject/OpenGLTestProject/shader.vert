#version 420
const int MAX_BONES = 100;
const int MAX_WEIGHTS = 3;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 TransformationMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main()
{
    gl_Position = ProjectionMatrix * ViewMatrix * TransformationMatrix * vec4(Position, 1.f);
}