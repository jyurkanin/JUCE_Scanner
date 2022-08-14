#include <JuceHeader.h>




class WaveTerrainWindow : public juce::OpenGLAppComponent{
public:
    WaveTerrainWindow();
    ~WaveTerrainWindow();
    
    void initialise() override;
    void render() override;
    void shutdown() override;
    
    void paint(juce::Graphics& g) override;
    
    
private:
    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix() const;
  
    juce::String vertexShader;
    juce::String fragmentShader;
  
    std::unique_ptr<juce::OpenGLShaderProgram> shader;

    //visual params
    static constexpr float width_scale = 1.0f;
    static constexpr float near_plane_dist = 8.5f;
    static constexpr float view_angle = .191;
    static constexpr float camera_height = .712f;
    
    static constexpr int gl_pos_idx = 0;
  //static constexpr int gl_color_idx = 1;
    static constexpr unsigned int points_per_wave = 100;
    static constexpr unsigned int max_waves = 100;
    static constexpr unsigned int max_vertices = points_per_wave*max_waves;
    
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    //position and colors
    float vertices[max_vertices*3];
    unsigned int num_waves;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveTerrainWindow)
};
