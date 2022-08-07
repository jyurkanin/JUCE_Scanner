#include "WaveTerrainWindow.h"
#include <stdio.h>
#include <math.h>

using namespace juce::gl;


WaveTerrainWindow::WaveTerrainWindow(){
    setSize(500,400);
    
    //Test.
    num_waves = max_waves;
    float temp_vertices[] = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };

    for(int i = 0; i < 12; i++){
        vertices[i] = temp_vertices[i];
    }
}

WaveTerrainWindow::~WaveTerrainWindow(){
  shutdownOpenGL();
}

void WaveTerrainWindow::initialise(){
  //printf("initialise current time %lld\n", juce::Time::getCurrentTime().toMilliseconds());
    bool result;
    bool is_good = true;
    
    juce::File cwd = juce::File::getCurrentWorkingDirectory();
    juce::String cwd_string = cwd.getParentDirectory().getParentDirectory().getFullPathName() + juce::String("/Source/shaders/");
    //juce::String temp_string = cwd_string + juce::String("wave_shader.frag");
    //printf("wave_shader.vert: %s\n", juce::File(cwd_string + juce::String("wave_shader.vert")).loadFileAsString().toRawUTF8());
    
    shader.reset(new juce::OpenGLShaderProgram(openGLContext));
    result = shader->addVertexShader(juce::File(cwd_string + juce::String("wave_shader.vert")).loadFileAsString());
    is_good &= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s\n", err.toRawUTF8());
    }
    
    result = shader->addFragmentShader(juce::File(cwd_string + juce::String("wave_shader.frag")).loadFileAsString());
    is_good &= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s\n", err.toRawUTF8());
    }
    
    result = shader->link();
    is_good &= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s\n", err.toRawUTF8());
    }
    
    if(!is_good){
        printf("[ERROR] opengl was not good\n");
        return;
    }
    
    shader->use();
    
    juce::String statusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    std::cout << statusText.toRawUTF8() << std::endl;
    
    
}

//jassert (getBounds().isEmpty() || ! isOpaque());


void WaveTerrainWindow::paint (juce::Graphics& g) {
  // You can add your component specific drawing code here!
  // This will draw over the top of the openGL background.

  g.setColour (getLookAndFeel().findColour (juce::Label::textColourId));
  g.setFont (20);
  g.drawText ("OpenGL Example", 25, 20, 300, 30, juce::Justification::left);
  g.drawLine (20, 20, 170, 20);
  g.drawLine (20, 50, 170, 50);
}


void WaveTerrainWindow::render(){
  //printf("render current time %lld\n", juce::Time::getCurrentTime().toMilliseconds());
  if(juce::OpenGLHelpers::isContextActive()){
    //printf("juce context is active\n");
  }
  else{
    printf("juce context is not active\n");
    return;
  }
  
  
  float desktopScale = openGLContext.getRenderingScale();
  juce::OpenGLHelpers::clear(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  
  glViewport(0, 0,
             juce::roundToInt(desktopScale * (float) getWidth()),
             juce::roundToInt(desktopScale * (float) getHeight()));

  shader->use();
  
  
  juce::Matrix3D<float> projectionMatrix = getProjectionMatrix();
  juce::Matrix3D<float> viewMatrix = getViewMatrix();
  shader->setUniformMat4("projectionMatrix", projectionMatrix.mat, 1, false);
  shader->setUniformMat4("viewMatrix", viewMatrix.mat, 1, false);
  
  
  glGenVertexArrays(1, &VAO); //Generates 1 vertex array object name
  glBindVertexArray(VAO);     //Binding a VAO makes it available for use
    
  //Create Buffer and create pointers to the attributes.
  ////This creates one buffer that looks like {pos,color,pos,color,...}
  //Just pos for now actually
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(gl_pos_idx, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    
  //no vertex color for now
  //glVertexAttribPointer(gl_color_idx, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
  //glEnableVertexAttribArray(gl_color_idx);



  
  unsigned int num_coords = 12; //3 points, 3 coords each
  unsigned int num_triangles = 2;
  unsigned int num_indices = num_triangles*3;
  unsigned int indices[num_indices] = {0,1,2,1,2,3};
  
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, num_coords*sizeof(float), vertices, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(float), indices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(gl_pos_idx);
  glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
  //glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(gl_pos_idx);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WaveTerrainWindow::shutdown(){
    shader.reset();
}





//Helper functions
juce::Matrix3D<float> WaveTerrainWindow::getProjectionMatrix() const {
    float w = 1.0f;// / (0.5f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio(false);
    return juce::Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}

juce::Matrix3D<float> WaveTerrainWindow::getViewMatrix() const {
    juce::Matrix3D<float> viewMatrix ({ 0.0f, 0.0f, -10.0f });
    juce::Matrix3D<float> rotationMatrix =
        viewMatrix.rotation ({-0.3f,
                              .01*getFrameCounter(),
                              0.0f});
    return rotationMatrix * viewMatrix;
}


