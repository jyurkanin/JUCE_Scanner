#include "WaveTerrainWindow.h"
#include "CompiledShaders.h"

#include <stdio.h>
#include <math.h>



using namespace juce::gl;


WaveTerrainWindow::WaveTerrainWindow(Scanner *s) : scanner(s){
    setSize(500,400);
    
    //Test.
    num_waves = max_waves;

    //zero the buffer. Kind of a stupid way to do it.
    for(int i = 0; i < points_per_wave; i++){
        wave_buffer[0][i] = 0;
        wave_buffer[1][i] = 0;
    }

    atomic_idx = 0;

    for(int i = 0; i < num_waves; i++){
        int offset = i*points_per_wave;
        for(int j = 0; j < points_per_wave; j++){
            float inc = (2*j) + 2*i;
            int v_offset = 3*(offset+j);
            vertices[v_offset+0] = width_scale*2* (-.5f + ((float)j/(points_per_wave-1)));
            vertices[v_offset+1] = .5f*sinf(juce::MathConstants<float>::pi*2*j/(points_per_wave-1));
            vertices[v_offset+2] = -10.0f*i/num_waves; //-1*logf((float)i);
        }
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
    //result = shader->addVertexShader(juce::File(cwd_string + juce::String("wave_shader.vert")).loadFileAsString());
    juce::String vertexString(CompiledShaders::Vertex);
    result = shader->addVertexShader(vertexString);
    is_good &= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Vertex Shader Error %s\n", err.toRawUTF8());
    }
    
    //result = shader->addFragmentShader(juce::File(cwd_string + juce::String("wave_shader.frag")).loadFileAsString());
    juce::String fragmentString(CompiledShaders::Fragment);
    result = shader->addFragmentShader(fragmentString);
    is_good &= result;
    if(!result){
        juce::String err = shader->getLastError();
        printf("Fragment Shader Error %s\n", err.toRawUTF8());
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

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

//jassert (getBounds().isEmpty() || ! isOpaque());


void WaveTerrainWindow::paint (juce::Graphics& g) {

}

void WaveTerrainWindow::updateVertices(){
    for(int i = num_waves-1; i > 0; i--){
        int offset = i*points_per_wave;
        
        for(int j = 0; j < points_per_wave; j++){
            int v_offset = 3*(offset+j);
            int prev_offset = v_offset - (3*points_per_wave);
            vertices[v_offset+1] = vertices[prev_offset+1];
        }
    }
    

    int cnt = 0;
    int temp_idx = scanner->buf_idx+1;
    for(int j = 0; j < points_per_wave; j++){
        int v_offset = 3*j;
        vertices[v_offset+1] = .5f*scanner->node_pos[temp_idx][cnt];
        cnt += 6;
    }

}

// void WaveTerrainWindow::updateWave(float *new_wave){
//     int cnt = 0;

//     int other_idx = atomic_idx ^ 1; //get the write index
//     for(int i = 0; i < points_per_wave; i++){
//         wave_buffer[other_idx][i] = new_wave[cnt];
//         cnt += ratio;
//     }
    
//     atomic_has_update = 1;
// }

void WaveTerrainWindow::render(){
    //printf("render current time %lld\n", juce::Time::getCurrentTime().toMilliseconds());
    if(juce::OpenGLHelpers::isContextActive()){
        //printf("juce context is active\n");
    }
    else{
        printf("juce context is not active\n");
        return;
    }
    
    // if(atomic_has_update){
    //     atomic_idx ^= 1;
    //     atomic_has_update = 0;

    if(getFrameCounter() % 4){
      updateVertices();
    }
    
    float desktopScale = openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear(juce::Colours::black);
    
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    
    
    glViewport(0, 0,
               juce::roundToInt(desktopScale * (float) getWidth()),
               juce::roundToInt(desktopScale * (float) getHeight()));
    
    shader->use();
    
    
    juce::Matrix3D<float> projectionMatrix = getProjectionMatrix();
    juce::Matrix3D<float> viewMatrix = getViewMatrix();
    shader->setUniformMat4("projectionMatrix", projectionMatrix.mat, 1, false);
    shader->setUniformMat4("viewMatrix", viewMatrix.mat, 1, false);
    
    unsigned int num_coords = 3*num_waves*points_per_wave; //number of vertices times 3
    unsigned int num_triangles = (points_per_wave-1)*(num_waves-1)*2;
    unsigned int num_indices = num_triangles*3;
    unsigned int indices[num_indices]; //reserve the largest chunk of memory that we might use.
    
    int cnt = 0;
    for(unsigned int i = 0; i < num_waves-1; i++){
        for(unsigned int j = 0; j < points_per_wave-1; j++){
            //we are iterating per square in the mesh.
            //each square has two triangles. Add them to the EBO.
            //bottom right triangle
            indices[cnt+0] = (i*points_per_wave) + j;
            indices[cnt+1] = (i*points_per_wave) + j + 1;
            indices[cnt+2] = ((i+1)*points_per_wave) + j + 1;
            cnt += 3;
            
            //upper left diagonal
            indices[cnt+0] = (i*points_per_wave) + j;
            indices[cnt+1] = ((i+1)*points_per_wave) + j;
            indices[cnt+2] = ((i+1)*points_per_wave) + j + 1;
            cnt += 3;
        }
    }
    
    

    glBindVertexArray(VAO);     //Binding a VAO makes it available for use
    
    //Create Buffer and create pointers to the attributes.
    ////This creates one buffer that looks like {pos,color,pos,color,...}
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_coords*sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(gl_pos_idx, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(float), indices, GL_STATIC_DRAW);
    
    
    glEnableVertexAttribArray(gl_pos_idx);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(gl_pos_idx);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WaveTerrainWindow::shutdown(){
    shader.reset();
}





//Helper functions
juce::Matrix3D<float> WaveTerrainWindow::getProjectionMatrix() const {
    float w = .5f;// / (0.5f + 0.1f);
    float h = w * getLocalBounds().toFloat().getAspectRatio(false);
    return juce::Matrix3D<float>::fromFrustum(-w, w, -h, h, near_plane_dist, 1000.0f);
}

juce::Matrix3D<float> WaveTerrainWindow::getViewMatrix() const {
    juce::Matrix3D<float> viewMatrix = juce::Matrix3D<float>::fromTranslation({0.0f, -camera_height, -near_plane_dist-10.0f});
    juce::Matrix3D<float> rotationMatrix = viewMatrix.rotation({
            view_angle,
            0, //.01*getFrameCounter(),
            0.0f});
    return rotationMatrix * viewMatrix;
}


