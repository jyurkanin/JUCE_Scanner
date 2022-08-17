#version 330 core
in vec3 vertexColor;
out vec4 FragColor;



void main() {
  float light = 11.5 - (16*gl_FragCoord.z);


  float greenness = .5 + vertexColor.x;
  float blueness = .5 -  vertexColor.x;
  
  FragColor = vec4(0, light*greenness, light*blueness, 1.0f);
  //FragColor = vec4(vec3(vertexColor.y), 1.0f);
  //FragColor = vec4(1.0f,0.0f,0.0f, 1.0f);
}
