#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <memory>

class Gallo {
public:
    enum class Estado {
        IDLE,
        CAMINANDO,
        COMIENDO,
        BLOQUEADO
    };
    
private:
    // Sprites y texturas
    sf::Vector2f m_escala;
    sf::Texture m_idleTexture;
    std::vector<sf::Texture> m_walkTextures;
    std::vector<sf::Texture> m_eatTextures;
    std::unique_ptr<sf::Sprite> m_sprite;
    
    // Estado
    Estado m_estado;
    sf::Vector2f m_position;
    float m_speed;
    bool m_mirandoDerecha;
    
    // Animación manual
    int m_frameActual;
    float m_tiempoFrame;
    float m_duracionFrame;
    
    // Movimiento
    float m_limiteIzquierdo;
    float m_limiteDerecho;
    
    // Temporizador para comer
    float m_tiempoComer;
    float m_tiempoEntreComidas;
    float m_duracionComida;

public:
    Gallo();
    void setLimites(float izquierda, float derecha);
    void setPosition(float x, float y);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    void verificarColisionJugador(const sf::FloatRect& playerBounds);
    
private:
    void cambiarEstado(Estado nuevo);
    void actualizarAnimacion(float dt);
};