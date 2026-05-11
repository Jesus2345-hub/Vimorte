#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>

struct Botella {
    std::unique_ptr<sf::Texture> texture;
    std::unique_ptr<sf::Sprite> sprite;
    sf::Color color;
    float animScale = 1.0f;
    bool animating = false;
};

struct Particula {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float life;
};

class MinigameColorMix {
private:
    bool m_isActive;
    bool m_gameWon;
    bool m_tieneColor;
    sf::Color m_colorActual;
    
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // Fondo
    sf::RectangleShape m_background;
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundSprite;
    
    // Botellas
    std::vector<std::unique_ptr<Botella>> m_botellas;
    
    // Papel para mezclar
    sf::RectangleShape m_papel;
    float m_papelAnimScale = 1.0f;
    
    // Botón reset
    sf::RectangleShape m_botonReset;
    
    // Colores
    std::vector<sf::Color> m_coloresDisponibles;
    std::vector<sf::Color> m_coloresObjetivo;
    std::vector<bool> m_completados;
    std::vector<sf::CircleShape> m_objetivosUI;
    std::vector<float> m_radiosOriginales;
    
    // Orden aleatorio
    std::vector<size_t> m_ordenObjetivos;
    size_t m_objetivoActual = 0;
    
    // Partículas
    std::vector<Particula> m_particulas;
    
    // UI
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_tituloObjetivos;
    std::unique_ptr<sf::Text> m_botonResetText;
    std::unique_ptr<sf::Text> m_instruccionEscText;
    std::unique_ptr<sf::Text> m_misionCumplida;
    std::unique_ptr<sf::Text> m_textoObjetivoActual;
    
    // Animaciones
    float m_tiempoCompletado = 0.0f;
    bool m_mostrandoCompletado = false;
    size_t m_ultimoCompletado = 0;
    
    sf::Color mezclar(const sf::Color& a, const sf::Color& b);
    void mezclarColor(const sf::Color& nuevo);
    void resetPapel();
    bool coloresIguales(const sf::Color& a, const sf::Color& b);
    void verificarColor();
    void generarOrdenAleatorio();
    void actualizarTextoObjetivo();
    void crearParticulas(sf::Vector2f pos, sf::Color color);
    
public:
    MinigameColorMix();
    void initUI();
    void setSize(const sf::Vector2f& size) { m_size = size; }
    void setPosition(const sf::Vector2f& pos) { m_position = pos; }
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    bool isGameWon() const { return m_gameWon; }
    void update(float dt);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    bool isCompleted() const { return m_gameWon; }
};