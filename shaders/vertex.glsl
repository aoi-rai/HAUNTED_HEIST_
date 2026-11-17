
#version 120

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec3 vNormal;
varying vec2 vTexCoord;

void main()
{
    vNormal = mat3(model) * normal;
    vTexCoord = texCoord;

    gl_Position = projection * view * model
                * vec4(position, 1.0);
}
