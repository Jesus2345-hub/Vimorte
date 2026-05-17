#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Animation {
private:
    std::vector<sf::Texture> m_frames;
    float m_frameDuration;
    float m_currentTime;
    int m_currentFrame;
    bool m_isPlaying;
    
public:
    Animation();
    
    void loadFromFolder(const std::string& basePath, int frameCount, float duration);
    void update(float dt);
    void applyToSprite(sf::Sprite& sprite);
    void play();
    void stop();
    void reset();
    bool isFinished() const;
    int getCurrentFrame() const;
};
