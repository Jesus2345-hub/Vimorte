#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Vimorte");
    
    sf::Font font;
    if (!font.openFromFile("assets/fonts/arial.ttf")) {
        std::cout << "Error: No se pudo cargar la fuente" << std::endl;
        return -1;
    }
    
    sf::Text text(font, "Hello Vimorte! Font working", 30);
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f(200, 500));
    
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(sf::Vector2f(375, 250));
    
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        
        window.clear(sf::Color::Black);
        window.draw(shape);
        window.draw(text);
        window.display();
    }
    
    return 0;
}
