#include "Animation.hpp"
#include <iostream>

Animation::Animation() 
    : m_frameDuration(0.1f), m_currentTime(0), m_currentFrame(0), m_isPlaying(true) {}

void Animation::loadFromFolder(const std::string& basePath, int frameCount, float duration) {
    m_frames.clear();
    m_frameDuration = duration / frameCount;
    m_currentFrame = 0;
    m_currentTime = 0;
    
    for (int i = 0; i < frameCount; ++i) {
        std::string filename = basePath + "_" + std::to_string(i) + ".png";
        sf::Texture texture;
        if (!texture.loadFromFile(filename)) {
            std::cerr << "Error: No se pudo cargar " << filename << std::endl;
            continue;
        }
        texture.setSmooth(false);

        m_frames.push_back(texture);
    }
    
    if (m_frames.empty()) {
        std::cerr << "Error: No se cargaron frames para " << basePath << std::endl;
    }
}

void Animation::update(float dt) {
    if (!m_isPlaying || m_frames.empty()) return;
    
    m_currentTime += dt;
    if (m_currentTime >= m_frameDuration) {
        m_currentTime = 0;
        m_currentFrame = (m_currentFrame + 1) % m_frames.size();
    }
}

void Animation::applyToSprite(sf::Sprite& sprite) {
    if (!m_frames.empty() && m_currentFrame < (int)m_frames.size()) {
        sprite.setTexture(m_frames[m_currentFrame]);
    }
}

void Animation::play() {
    m_isPlaying = true;
}

void Animation::stop() {
    m_isPlaying = false;
}

void Animation::reset() {
    m_currentFrame = 0;
    m_currentTime = 0;
}

bool Animation::isFinished() const {
    return !m_isPlaying;
}

int Animation::getCurrentFrame() const {
    return m_currentFrame;
}
