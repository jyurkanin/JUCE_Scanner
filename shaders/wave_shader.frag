#version 330 core
in vec3 vertexColor;
out vec4 FragColor;



void main() {
  float dark = 1-(gl_FragCoord.z);
  FragColor = vec4(dark*vertexColor.x, 0, dark, 1.0f);
  //FragColor = vec4(vec3(vertexColor.y), 1.0f);
  //FragColor = vec4(1.0f,0.0f,0.0f, 1.0f);
}
