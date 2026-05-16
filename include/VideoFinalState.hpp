#pragma once
#include "State.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <vector>

class Game;

class VideoFinalState : public State {
private:

    void updateSpriteScale();
    std::vector<sf::Texture> m_frames;
    std::unique_ptr<sf::Sprite> m_videoSprite;
    sf::Clock m_frameClock;
    sf::Music m_music;
    
    int m_currentFrame;
    float m_frameDuration;
    bool m_isGoodEnding;
    std::string m_videoPath;
    
    // campos para la introducción
    bool m_isIntro;
    int m_introSlotId;
    std::string m_introNombreJugador;
    
public:
    // Constructor para videos finales normales
    VideoFinalState(sf::RenderWindow* window, Game* game, 
                    const std::string& videoFolder, bool isGoodEnding);
    
    // Constructor para introducción
    VideoFinalState(sf::RenderWindow* window, Game* game, 
                    const std::string& videoFolder, int slotId, const std::string& nombreJugador);
    
    ~VideoFinalState() override = default;
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};