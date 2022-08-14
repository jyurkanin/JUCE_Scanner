#include <JuceHeader.h>




class WaveTerrainWindow : public juce::OpenGLAppComponent{
public:
    WaveTerrainWindow();
    ~WaveTerrainWindow();
    
    void initialise() override;
    void render() override;
    void shutdown() override;
    
    void paint(juce::Graphics& g) override;

    float near_plane_dist = 7.0f;
    
private:
    juce::Matrix3D<float> getProjectionMatrix() const;
    juce::Matrix3D<float> getViewMatrix() const;
  
    juce::String vertexShader;
    juce::String fragmentShader;
  
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    
    
    static constexpr int gl_pos_idx = 0;
    static constexpr int gl_color_idx = 1;
    static constexpr unsigned int points_per_wave = 100;
    static constexpr unsigned int max_waves = 100;
    static constexpr unsigned int max_vertices = points_per_wave*max_waves;
    
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    //position and colors
    float vertices[max_vertices*6];
    unsigned int num_waves;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveTerrainWindow)
};
