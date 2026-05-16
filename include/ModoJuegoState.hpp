#ifndef MODOJUEGOSTATE_HPP
#define MODOJUEGOSTATE_HPP

#include "State.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class ModoJuegoState : public State {
private:
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    std::unique_ptr<sf::Text> m_title;
    std::unique_ptr<sf::Text> m_description;
    
    std::unique_ptr<sf::Text> m_opcionAgradableTitulo;
    std::vector<sf::Text> m_opcionAgradableLineas;  // Múltiples líneas para wrapping
    std::unique_ptr<sf::Text> m_opcionConsecuenciasTitulo;
    std::vector<sf::Text> m_opcionConsecuenciasLineas;  // Múltiples líneas para wrapping
    
    std::unique_ptr<sf::Text> m_advertencia;
    
    sf::RectangleShape m_botonAgradable;
    sf::RectangleShape m_botonConsecuencias;
    
    bool m_hoverAgradable = false;
    bool m_hoverConsecuencias = false;
    
    std::string m_nombreJugador;
    int m_slotId;
    
    // Base resolution para referencia
    const float BASE_WIDTH = 1280.0f;
    const float BASE_HEIGHT = 720.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    
    void updateScale();
    int getScaledFontSize(int baseSize);
    float getScaledValue(float baseValue);
    
    // Función para dividir texto en líneas
    std::vector<std::string> wrapText(const std::string& text, int maxWidth, int fontSize);
    void rebuildTexts();
    
public:
    ModoJuegoState(sf::RenderWindow* window, Game* game, const std::string& nombre, int slotId);
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};

#endif