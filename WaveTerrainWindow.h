#include <JuceHeader.h>

#include <atomic>

#include "scanner.h"


#pragma once


class WaveTerrainWindow : public juce::OpenGLAppComponent{
public:
    WaveTerrainWindow(Scanner *s);
    ~WaveTerrainWindow();
    
    void initialise() override;
    void render() override;
    void shutdown() override;
    void paint(juce::Graphics& g) override;

    //void updateWave(float *new_wave);
    void updateVertices();
    
private:
    const Scanner *scanner;
    
    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix() const;
  
    juce::String vertexShader;
    juce::String fragmentShader;
  
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::atomic<unsigned> atomic_idx;
    std::atomic<unsigned> atomic_has_update;
    
    
    //visual params
    static constexpr float width_scale = 1.0f;
    static constexpr float near_plane_dist = 8.5f; //8.5
    static constexpr float view_angle = .161; //.191
    static constexpr float camera_height = .612f; //.712
    
    static constexpr int gl_pos_idx = 0;
  //static constexpr int gl_color_idx = 1;
    static constexpr unsigned int ratio = 6;
    static constexpr unsigned int points_per_wave = Scanner::num_nodes / ratio;
    static constexpr unsigned int max_waves = 100;
    static constexpr unsigned int max_vertices = points_per_wave*max_waves;
    
    float wave_buffer[2][points_per_wave];
    
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    //position and colors
    float vertices[max_vertices*3];
    unsigned int num_waves;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveTerrainWindow)
};
