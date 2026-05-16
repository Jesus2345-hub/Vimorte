#include "VideoFinalState.hpp"
#include "Game.hpp"
#include "ModoJuegoState.hpp"
#include "LevelTree.hpp"
#include <iostream>
#include <filesystem>

// Constructor para videos finales (centinelas)
VideoFinalState::VideoFinalState(sf::RenderWindow* window, Game* game, 
                                 const std::string& videoFolder, bool isGoodEnding)
    : State(window, game)
    , m_currentFrame(0)
    , m_frameDuration(3.0f)
    , m_isGoodEnding(isGoodEnding)
    , m_videoPath(videoFolder)
    , m_isIntro(false)
    , m_introSlotId(-1)
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
    
    if (m_frames.empty()) {
        std::cerr << "ERROR: No se encontraron frames en: " << videoFolder << std::endl;
        return;
    }
    
    m_videoSprite = std::make_unique<sf::Sprite>(m_frames[0]);
    updateSpriteScale();  
    m_videoSprite->setPosition(sf::Vector2f(0.f, 0.f));
    
    std::string musicPath = videoFolder + "/music.ogg";
    if (std::filesystem::exists(musicPath)) {
        if (m_music.openFromFile(musicPath)) {
            m_music.setLooping(false);
            m_music.play();
        }
    }
    
    m_frameClock.restart();
    std::cout << "Video final iniciado. Frames: " << m_frames.size() << std::endl;
}

// Constructor para introduccion 
VideoFinalState::VideoFinalState(sf::RenderWindow* window, Game* game, 
                                 const std::string& videoFolder, int slotId, const std::string& nombreJugador)
    : State(window, game)
    , m_currentFrame(0)
    , m_frameDuration(2.0f)
    , m_isGoodEnding(true)
    , m_videoPath(videoFolder)
    , m_isIntro(true)
    , m_introSlotId(slotId)
    , m_introNombreJugador(nombreJugador)
{
    m_videoSprite = nullptr;
    
    for (int i = 1; i <= 8; i++) {
        std::string framePath = videoFolder + "/frame_" + std::to_string(i) + ".png";
        
        if (std::filesystem::exists(framePath)) {
            sf::Texture texture;
            if (texture.loadFromFile(framePath)) {
                m_frames.push_back(std::move(texture));
                std::cout << "Cargado frame de intro: " << framePath << std::endl;
            }
        }
    }
    
    if (m_frames.empty()) {
        std::cerr << "ERROR: No se encontraron frames en: " << videoFolder << std::endl;
        game->changeState(std::make_unique<ModoJuegoState>(window, game, nombreJugador, slotId));
        return;
    }
    
    m_videoSprite = std::make_unique<sf::Sprite>(m_frames[0]);
    updateSpriteScale();  
    m_videoSprite->setPosition(sf::Vector2f(0.f, 0.f));
    
    std::string musicPath = videoFolder + "/music.ogg";
    if (std::filesystem::exists(musicPath)) {
        if (m_music.openFromFile(musicPath)) {
            m_music.setLooping(false);
            m_music.play();
        }
    }
    
    m_frameClock.restart();
    std::cout << "Video de introduccion iniciado (" << m_frames.size() << " frames)" << std::endl;
}


void VideoFinalState::updateSpriteScale() {
    if (!m_videoSprite) return;
    
    sf::Vector2u winSize = window->getSize();
    
    const sf::Texture& texture = m_videoSprite->getTexture();
    sf::Vector2u texSize = texture.getSize();
    
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = static_cast<float>(winSize.x) / static_cast<float>(texSize.x);
        float scaleY = static_cast<float>(winSize.y) / static_cast<float>(texSize.y);
        m_videoSprite->setScale({scaleX, scaleY});
    }
}

void VideoFinalState::update(float dt) {
    if (!m_videoSprite || m_frames.empty()) return;
    
    if (m_frameClock.getElapsedTime().asSeconds() >= m_frameDuration) {
        m_currentFrame++;
        
        if (m_currentFrame >= static_cast<int>(m_frames.size())) {
            // VIDEO TERMINADO
            if (m_isIntro) {
                std::cout << "Introduccion terminada. Pasando a eleccion de modo..." << std::endl;
                game->changeState(std::make_unique<ModoJuegoState>(window, game, m_introNombreJugador, m_introSlotId));
            } else {
                std::cout << "Video final terminado. Volviendo al menu..." << std::endl;
                game->returnToMenu();
            }
            return;
        }
        
        // Cambiar la textura y recalcular escala para este frame específico
        m_videoSprite->setTexture(m_frames[m_currentFrame]);
        updateSpriteScale();  // Recalcular escala para la nueva textura
        m_videoSprite->setPosition(sf::Vector2f(0.f, 0.f));
        
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
            
            m_music.stop();
            
            if (m_isIntro) {
                std::cout << "ESC presionado durante introducción. Volviendo al menú..." << std::endl;
                game->returnToMenu();
            } else {
                game->returnToMenu();
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Space ||
                 keyPressed->code == sf::Keyboard::Key::Enter) {
            
            m_music.stop();
            
            if (m_isIntro) {
                game->changeState(std::make_unique<ModoJuegoState>(window, game, m_introNombreJugador, m_introSlotId));
            } else {
                game->returnToMenu();
            }
        }
    }
}