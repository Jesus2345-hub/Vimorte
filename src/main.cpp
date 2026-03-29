#include <SFML/Graphics.hpp>
#include "test/Prueba.hpp"

int main() {
    Prueba prueba;
    prueba.saludar();
    
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Vimorte");
    
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        
        window.clear();
        window.draw(shape);
        window.display();
    }
    
    return 0;
}
