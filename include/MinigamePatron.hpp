#ifndef MINIGAMEPATRON_HPP
#define MINIGAMEPATRON_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <random>

class MinigamePatron {
public:
    MinigamePatron();
    
    void setSize(const sf::Vector2f& size);
    void setPosition(const sf::Vector2f& position);
    void activate();
    void deactivate();
    bool isActive() const { return m_active; }
    bool isCompleted() const { return m_completed; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }

private:
    bool m_active = false;
    bool m_completed = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // Patrón objetivo (ROJO=0, AZUL=1, VERDE=2)
    std::vector<int> m_patronObjetivo = {0, 1, 2};
    std::vector<int> m_patronJugador;
    int m_pasoActual = 0;
    
    // Círculos de colores disponibles para elegir
    struct BotonColor {
        sf::CircleShape circulo;
        sf::Color color;
        std::string nombre;
        int indiceColor = -1;
        bool hover = false;
    };
    std::vector<BotonColor> m_botonesColores;
    
    // Círculos que muestran el patrón objetivo (en la "pared")
    std::vector<sf::CircleShape> m_muestraPatron;
    
    // Círculos que muestran lo que el jugador lleva seleccionado
    std::vector<sf::CircleShape> m_progresoJugador;
    
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInstrucciones;
    std::unique_ptr<sf::Text> m_textoEstado;
    std::unique_ptr<sf::Text> m_textoPatron;
    
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    // Textura de fondo
    sf::Texture m_texturaFondo;
    std::unique_ptr<sf::Sprite> m_spriteFondo;
    bool m_fondoCargado = false;
    
    std::function<void()> m_onComplete;
    
    std::mt19937 m_gen;
    
    void inicializarBotones();
    void seleccionarColor(int indice);
    void verificarPatron();
    void reiniciarPatron();
    sf::Color obtenerColor(int indice) const;
    std::string obtenerNombreColor(int indice) const;
    
    void actualizarPosiciones();
};

#endif