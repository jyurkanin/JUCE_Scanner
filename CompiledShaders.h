#include <JuceHeader.h>

class CompiledShaders{
public:
  static constexpr char* Vertex = R"DERP(#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
//layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 vertexColor; // output a color to the fragment shader


void main() {
    gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
    //gl_Position = vec4(aPos, 1.0);
    vertexColor = vec3(1.0f*(aPos.y) - .2, 0, 0);
})DERP";
  static constexpr char* Fragment = R"HERP(#version 330 core
in vec3 vertexColor;
out vec4 FragColor;



void main() {
  float light = 11.5 - (16*gl_FragCoord.z);


  float greenness = .5 + vertexColor.x;
  float blueness = .5 -  vertexColor.x;
  
  FragColor = vec4(0, light*greenness, light*blueness, 1.0f);
  //FragColor = vec4(vec3(vertexColor.y), 1.0f);
  //FragColor = vec4(1.0f,0.0f,0.0f, 1.0f);
})HERP";
};
