#pragma once
#include <SFML/Graphics.hpp>
#include <memory> // Para std::unique_ptr

class Menu
{
public:
    Menu(float ancho, float alto);
    void actualizar(sf::Vector2i mousePos);
    void dibujar(sf::RenderWindow& ventana);
    bool verificarClick(sf::Vector2i mousePos);
    bool verificarClickSalir(sf::Vector2i mousePos);
    bool verificarClickConfig(sf::Vector2i mousePos);

private:
    sf::Texture texturaFondo;
    std::unique_ptr<sf::Sprite> spriteFondo; // Puntero

    sf::Texture texturaConfig;
    std::unique_ptr<sf::Sprite> spriteConfig; // Puntero

    sf::Font fuente;
    std::unique_ptr<sf::Text> textoBoton; // Puntero
    std::unique_ptr<sf::Text> textoSalir; // Puntero

    sf::RectangleShape cajaBoton;
    sf::RectangleShape cajaSalir;
    sf::RectangleShape cajaConfig;
};