#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>

struct Botella {
    std::unique_ptr<sf::Sprite> sprite;
    std::unique_ptr<sf::Texture> texture;
    sf::Color color;

    Botella() : color(sf::Color::White) {}
    
    Botella(Botella&&) noexcept = default;
    Botella& operator=(Botella&&) noexcept = default;
    Botella(const Botella&) = delete;
    Botella& operator=(const Botella&) = delete;
};

class MinigameColorMix {
public:
    MinigameColorMix();
    
    void initUI();
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);

    void activate() { m_isActive = true; initUI(); }
    void deactivate() { m_isActive = false; }

    bool isActive() const { return m_isActive; }
    bool isWon() const { return m_gameWon; }

    void setPosition(sf::Vector2f pos) { m_position = pos; }
    void setSize(sf::Vector2f size) { 
        m_size = size;
        m_background.setSize(size);
    }

private:
    sf::Color mezclar(const sf::Color& a, const sf::Color& b);
    void mezclarColor(const sf::Color& nuevo);
    void resetPapel();
    bool coloresIguales(const sf::Color& a, const sf::Color& b);
    void verificarColor();

    bool m_isActive;
    bool m_gameWon;
    bool m_tieneColor;

    sf::Vector2f m_position;
    sf::Vector2f m_size;                   

    sf::RectangleShape m_background;
    sf::RectangleShape m_papel;
    sf::RectangleShape m_botonReset;
    
    std::vector<sf::CircleShape> m_objetivosUI;
    std::vector<float> m_radiosOriginales;

    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundSprite;  

    std::vector<std::unique_ptr<Botella>> m_botellas;
    std::vector<sf::CircleShape> m_botonesColores;

    std::vector<sf::Color> m_coloresDisponibles;
    std::vector<sf::Color> m_coloresObjetivo;
    std::vector<bool> m_completados;

    sf::Color m_colorActual;

    // Textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_botonResetText;
    std::unique_ptr<sf::Text> m_instruccionEscText;  
    std::unique_ptr<sf::Text> m_tituloObjetivos; 
    std::unique_ptr<sf::Text> m_misionCumplida; 
};