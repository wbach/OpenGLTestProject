#version 440
const int MAX_BONES = 100;
const int MAX_WEIGHTS = 3;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

// layout(std140, binding=0) uniform PerApp
// {
//     vec4 color;
// } inputData;

layout(std140, align=16,  binding=1) uniform PerResize
{
     bool swapDir;
     vec3 color;
     mat4 ProjectionMatrix;
} perResize;

layout(std140, binding=2) uniform PerFrame
{
    mat4 ViewMatrix;
} perFrame;

layout(std140, binding=3) uniform PerObject
{
    mat4 TransformationMatrix;
} perObject;

out vec4 color;

void main()
{
    vec4 pos = perResize.ProjectionMatrix * perFrame.ViewMatrix * perObject.TransformationMatrix * vec4(Position, 1.f);
    color = vec4(perResize.color,1.f);
    if (perResize.swapDir)
    {
        pos.x = -pos.x;
    }
    gl_Position = pos;
}