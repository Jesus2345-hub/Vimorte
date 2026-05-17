#include "Estados/VideoFinalState.hpp"
#include "Configuracion/Game.hpp"
#include "Estados/ModoJuegoState.hpp"
#include "Estados/CaminoFinalState.hpp"     // Esta linea es nueva
#include "Estructuras/LevelTree.hpp"
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
    m_lastWindowSize = window->getSize();
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
    m_lastWindowSize = window->getSize();
    m_frameClock.restart();
    std::cout << "Video de introduccion iniciado (" << m_frames.size() << " frames)" << std::endl;
}


void VideoFinalState::updateSpriteScale() {
    if (!m_videoSprite) return;
    
    sf::Vector2u winSize = window->getSize();
    const sf::Texture& texture = m_videoSprite->getTexture();
    sf::Vector2u texSize = texture.getSize();
    
    if (texSize.x > 0 && texSize.y > 0) {
        // Mantener aspect ratio - LETTERBOX
        float winAspect = static_cast<float>(winSize.x) / static_cast<float>(winSize.y);
        float texAspect = static_cast<float>(texSize.x) / static_cast<float>(texSize.y);
        
        float scale;
        if (winAspect > texAspect) {
            // Pantalla más ancha - escalar por altura
            scale = static_cast<float>(winSize.y) / static_cast<float>(texSize.y);
        } else {
            // Pantalla más alta - escalar por ancho
            scale = static_cast<float>(winSize.x) / static_cast<float>(texSize.x);
        }
        
        m_videoSprite->setScale({scale, scale});
        
        // Centrar en pantalla
        sf::FloatRect bounds = m_videoSprite->getLocalBounds();
        float scaledWidth = bounds.size.x * scale;
        float scaledHeight = bounds.size.y * scale;
        
        m_videoSprite->setPosition({
            (winSize.x - scaledWidth) / 2.f,
            (winSize.y - scaledHeight) / 2.f
    });
    }
}

void VideoFinalState::update(float dt) {
    
    if (!m_videoSprite || m_frames.empty()) return;
    
    // Verificar si la ventana cambió de tamaño
    sf::Vector2u currentSize = window->getSize();
    if (currentSize != m_lastWindowSize) {
        m_lastWindowSize = currentSize;
        updateSpriteScale();
        // Reposicionar después de escalar
        sf::FloatRect bounds = m_videoSprite->getLocalBounds();
        float scaledWidth = bounds.size.x * m_videoSprite->getScale().x;
        float scaledHeight = bounds.size.y * m_videoSprite->getScale().y;
        m_videoSprite->setPosition({
            (currentSize.x - scaledWidth) / 2.f,
            (currentSize.y - scaledHeight) / 2.f
        });
    }
    if (m_frameClock.getElapsedTime().asSeconds() >= m_frameDuration) {
        m_currentFrame++;
        
        if (m_currentFrame >= static_cast<int>(m_frames.size())) {
            // VIDEO TERMINADO
            if (m_isIntro) {
                // Si es la introduccion del juego, pasar directamente a elegir modo
                std::cout << "Introduccion terminada. Pasando a eleccion de modo..." << std::endl;
                game->changeState(std::make_unique<ModoJuegoState>(window, game, m_introNombreJugador, m_introSlotId));
                        } else {
                std::cout << "Video final terminado. Mostrando el camino recorrido..." << std::endl;
                
                // Obtener el identificador del nodo actual (el final alcanzado)
                std::string identificadorFinal = game->getLevelTree().obtenerIdNodoActual();
                
                // Mostrar informacion de depuracion
                std::cout << "Final alcanzado: " << identificadorFinal << std::endl;
                std::cout << "Nivel padre del centinela: " 
                          << game->getLevelTree().obtenerNivelPadreDelCentinela() 
                          << std::endl;
                
                // Crear el estado que muestra el camino recorrido en el arbol
                auto estadoCamino = std::make_unique<CaminoFinalState>(
                    window, 
                    game, 
                    identificadorFinal
                );
                
                // Cambiar al estado del camino
                game->changeState(std::move(estadoCamino));
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
 //empty
}