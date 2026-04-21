#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Menu
{
public:
    Menu(float ancho, float alto);
    void actualizar(sf::Vector2i mousePos);
    void dibujar(sf::RenderWindow& ventana);
    
    // Botones principales
    bool verificarClickJugar(sf::Vector2i mousePos);
    bool verificarClickCargar(sf::Vector2i mousePos);
    bool verificarClickSalir(sf::Vector2i mousePos);
    bool verificarClickConfig(sf::Vector2i mousePos);
    
    const sf::Font& getFuente() const { return fuente; }

private:
    sf::Texture texturaFondo;
    std::unique_ptr<sf::Sprite> spriteFondo;

    sf::Texture texturaConfig;
    std::unique_ptr<sf::Sprite> spriteConfig;

    sf::Font fuente;
    
    // Textos de botones
    std::unique_ptr<sf::Text> textoJugar;
    std::unique_ptr<sf::Text> textoCargar;
    std::unique_ptr<sf::Text> textoSalir;

    // Cajas de botones
    sf::RectangleShape cajaJugar;
    sf::RectangleShape cajaCargar;
    sf::RectangleShape cajaSalir;
    sf::RectangleShape cajaConfig;
};