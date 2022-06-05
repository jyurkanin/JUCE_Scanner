#include "WaveTerrainWindow.h"
#include <stdio.h>





WaveTerrainWindow::WaveTerrainWindow(){
    setSize(500,400);
}

WaveTerrainWindow::~WaveTerrainWindow(){
    
}

void WaveTerrainWindow::initialise(){
    bool result;
    bool failed = false;
    
    shader.reset(new juce::OpenGLShaderProgram(openGLContext));
    result = shader->addVertexShader(juce::File("./shaders/wave_shader.vert").loadFileAsString());
    failed |= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s", err.toRawUTF8());
    }
    
    result = shader->addFragmentShader(juce::File("./shaders/wave_shader.frag").loadFileAsString());
    failed |= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s", err.toRawUTF8());
    }

    result = shader->link();
    failed |= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s", err.toRawUTF8());
    }
    
    if(failed){
        return;
    }
    
    shape.reset(new juce::Shape());
    attributes.reset(new juce::Attributes(*shader));
    uniforms.reset(new juce::Uniforms(*shader));
    
    shader->use();
    
    statusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    
    
}

void WaveTerrainWindow::render(){
    
}

void WaveTerrainWindow::shutdown(){
    shader.reset();
    shape.reset();
    attributes.reset();
    uniforms.reset();
}

void WaveTerrainWindow::shutdownOpenGL(){
    
}






//Helper functions
juce::Matrix3D<float> WaveTerrainWindow::getProjectionMatrix() const {
    float w = 1.0f / (0.5f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio(false);
    return juce::Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}

juce::Matrix3D<float> WaveTerrainWindow::getViewMatrix() const {
    juce::Matrix3D<float> viewMatrix ({ 0.0f, 0.0f, -10.0f });
    juce::Matrix3D<float> rotationMatrix =
        viewMatrix.rotation ({-0.3f,
                              5.0f * std::sin ((float) getFrameCounter() * 0.01f),
                              0.0f});
    return rotationMatrix * viewMatrix;
}


