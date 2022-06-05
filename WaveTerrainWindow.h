#include <JuceHeader.h>
#include "WaveTerrainWindow.h"


class WaveTerrainWindow : public juce::OpenGLAppComponent{
public:
    WaveTerrainWindow();
    ~WaveTerrainWindow();
    
    void initialise() override;
    void render() override;
    void shutdown() override;
    void shutdownOpenGL() override;
    
    
private:
    juce::Matrix3D<float> WaveTerrainWindow::getProjectionMatrix() const;
    juce::Matrix3D<float> WaveTerrainWindow::getViewMatrix() const;
    
    juce::String vertexShader;
    juce::String fragmentShader;
    
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<juce::Shape> shape;
    std::unique_ptr<juce::Attributes> attributes;
    std::unique_ptr<juce::Uniforms> uniforms;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveTerrainWindow)
};
