#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 vColor; // output a color to the fragment shader


void main() {
    vColor = aColor; // set ourColor to the input color we got from the vertex data
    gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
}
