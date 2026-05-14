#include "VideoFinalState.hpp"
#include "Game.hpp"
#include <iostream>
#include <filesystem>

VideoFinalState::VideoFinalState(sf::RenderWindow* window, Game* game, 
                                 const std::string& videoFolder, bool isGoodEnding)
    : State(window, game)
    , m_currentFrame(0)
    , m_frameDuration(2.0f)
    , m_isGoodEnding(isGoodEnding)
    , m_videoPath(videoFolder)
{
    m_videoSprite = nullptr;
    
    for (int i = 1; i <= 8; i++) {
        std::string framePath = videoFolder + "/frame_" + std::to_string(i) + ".png";
        
        if (std::filesystem::exists(framePath)) {
            sf::Texture texture;
            if (texture.loadFromFile(framePath)) {
                m_frames.push_back(std::move(texture));
                std::cout << "Cargado: " << framePath << std::endl;
            }
        }
    }
    
    // Si no hay frames, mostrar error y salir
    if (m_frames.empty()) {
        std::cerr << "ERROR: No se encontraron frames en: " << videoFolder << std::endl;
        return;
    }
    
    // Crear sprite con el primer frame
    m_videoSprite = std::make_unique<sf::Sprite>(m_frames[0]);
    
    // Escalar a la ventana
    sf::Vector2u winSize = window->getSize();
    sf::Vector2u texSize = m_frames[0].getSize();
    
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = static_cast<float>(winSize.x) / static_cast<float>(texSize.x);
        float scaleY = static_cast<float>(winSize.y) / static_cast<float>(texSize.y);
        m_videoSprite->setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    // Música opcional
    std::string musicPath = videoFolder + "/music.ogg";
    if (std::filesystem::exists(musicPath)) {
        m_music.openFromFile(musicPath);
        m_music.setLooping(false);
        m_music.play();
    }
    
    m_frameClock.restart();
    std::cout << "Video iniciado. Frames: " << m_frames.size() << std::endl;
}

void VideoFinalState::update(float dt) {
    if (!m_videoSprite || m_frames.empty()) return;
    
    if (m_frameClock.getElapsedTime().asSeconds() >= m_frameDuration) {
        m_currentFrame++;
        
        if (m_currentFrame >= static_cast<int>(m_frames.size())) {
            // Terminar video...
            return;
        }
        
        m_videoSprite->setTexture(m_frames[m_currentFrame]);
        
        // RE-ESCALAR después de cambiar la textura
        sf::Vector2u winSize = window->getSize();
        sf::Vector2u texSize = m_frames[m_currentFrame].getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = static_cast<float>(winSize.x) / static_cast<float>(texSize.x);
            float scaleY = static_cast<float>(winSize.y) / static_cast<float>(texSize.y);
            m_videoSprite->setScale(sf::Vector2f(scaleX, scaleY));
            // Centrar en la pantalla
            m_videoSprite->setPosition(sf::Vector2f(0.f, 0.f));
        }
        
        m_frameClock.restart();
    }
}
void VideoFinalState::draw() {
    if (m_videoSprite) {
        window->draw(*m_videoSprite);
    }
}

void VideoFinalState::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            if (game) {
                LevelTree& levelTree = game->getLevelTree();
                if (levelTree.jumpToNode("nivel1")) {
                    auto newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            }
        }
    }
}