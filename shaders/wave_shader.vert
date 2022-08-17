#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
//layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 vertexColor; // output a color to the fragment shader


void main() {
    gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
    //gl_Position = vec4(aPos, 1.0);
    vertexColor = vec3(1.0f*(aPos.y) - .2, 0, 0);
}
