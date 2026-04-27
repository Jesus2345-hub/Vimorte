#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <memory>

class Menu {
private:
    sf::Texture texturaFondo;
    sf::Texture texturaConfig;
    sf::Font fuente;
    
    std::unique_ptr<sf::Sprite> spriteFondo;
    std::unique_ptr<sf::Sprite> spriteConfig;
    std::unique_ptr<sf::Text> textoJugar;
    std::unique_ptr<sf::Text> textoSalir;
    
    sf::RectangleShape cajaJugar;
    sf::RectangleShape cajaSalir;
    sf::RectangleShape cajaConfig;
    
public:
    Menu(float ancho, float alto);
    
    void redimensionar(float ancho, float alto);
    void actualizar(sf::Vector2i mousePos);
    void dibujar(sf::RenderWindow& ventana);
    
    bool verificarClickJugar(sf::Vector2i mousePos);
    bool verificarClickSalir(sf::Vector2i mousePos);
    bool verificarClickConfig(sf::Vector2i mousePos);
    
    sf::Font& getFuente() { return fuente; }
};

#endif