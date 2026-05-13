#include "VideoFinalState.hpp"
#include "Game.hpp"
#include <iostream>
#include <filesystem>
#include <cstdint>
#include <array>

VideoFinalState::VideoFinalState(sf::RenderWindow* window, Game* game, 
                                 const std::string& videoFolder, bool isGoodEnding)
    : State(window, game)
    , m_currentFrame(0)
    , m_frameDuration(0.05f)
    , m_isGoodEnding(isGoodEnding)
    , m_videoPath(videoFolder)
{
    // En SFML 3.0.2, crear un sprite sin textura no es posible
    // En lugar de eso, creamos el sprite como nullptr y lo asignamos después
    m_videoSprite = nullptr;
    
    // Cargar frames del video
    int frameIndex = 0;
    while (true) {
        std::string framePath = videoFolder + "/frame_" + std::to_string(frameIndex) + ".png";
        
        if (!std::filesystem::exists(framePath)) {
            break;
        }
        
        sf::Texture texture;
        if (texture.loadFromFile(framePath)) {
            m_frames.push_back(std::move(texture));
            std::cout << "Cargado frame: " << framePath << std::endl;
        } else {
            std::cerr << "Error cargando frame: " << framePath << std::endl;
        }
        
        frameIndex++;
    }
    
    // Si hay frames, crear el sprite con el primer frame
    if (!m_frames.empty()) {
        m_videoSprite = std::make_unique<sf::Sprite>(m_frames[0]);
        
        sf::Vector2u winSize = window->getSize();
        sf::Vector2u texSize = m_frames[0].getSize();
        
        if (texSize.x > 0 && texSize.y > 0) {
            float scaleX = static_cast<float>(winSize.x) / static_cast<float>(texSize.x);
            float scaleY = static_cast<float>(winSize.y) / static_cast<float>(texSize.y);
            m_videoSprite->setScale(sf::Vector2f(scaleX, scaleY));
        }
    } else {
        std::cerr << "Error: No se encontraron frames de video en: " << videoFolder << std::endl;
        
        // Fallback: crear un sprite con una textura simple si existe
        sf::Texture fallback;
        if (fallback.loadFromFile("assets/images/fallback.png")) {
            m_videoSprite = std::make_unique<sf::Sprite>(fallback);
        }
    }
    
    // Cargar música
    std::string musicPath = videoFolder + "/music.ogg";
    if (std::filesystem::exists(musicPath)) {
        if (!m_music.openFromFile(musicPath)) {
            std::cerr << "Error cargando música: " << musicPath << std::endl;
        } else {
            m_music.setLooping(false);
            m_music.play();
        }
    }
    
    m_frameClock.restart();
}

void VideoFinalState::update(float dt) {
    if (m_videoSprite && m_frameClock.getElapsedTime().asSeconds() >= m_frameDuration) {
        m_currentFrame++;
        
        if (m_currentFrame >= static_cast<int>(m_frames.size())) {
            if (game) {
                LevelTree& levelTree = game->getLevelTree();
                if (levelTree.jumpToNode("menu")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            }
            return;
        }
        
        if (m_currentFrame < static_cast<int>(m_frames.size())) {
            m_videoSprite->setTexture(m_frames[m_currentFrame]);
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
        if (keyPressed->code == sf::Keyboard::Key::Escape || 
            keyPressed->code == sf::Keyboard::Key::Space ||
            keyPressed->code == sf::Keyboard::Key::Enter) {
            if (game) {
                LevelTree& levelTree = game->getLevelTree();
                if (levelTree.jumpToNode("menu")) {
                    std::unique_ptr<State> newState = levelTree.createCurrentState(window, game);
                    if (newState) {
                        game->changeState(std::move(newState));
                    }
                }
            }
        }
    }
}