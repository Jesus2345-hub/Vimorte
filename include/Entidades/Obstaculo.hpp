#pragma once
#include <SFML/Graphics.hpp>

class Obstaculo 
{
private:
    sf::FloatRect m_caja;

public:
    Obstaculo(float x, float y, float ancho, float alto) 
        : m_caja({x, y}, {ancho, alto}){}

    // Obtener caja
    sf::FloatRect getBounds() const { return m_caja; }
};