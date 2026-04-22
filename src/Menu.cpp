#include "Menu.hpp"
#include <iostream>

Menu::Menu(float ancho, float alto)
{
    // 1. CARGAR ARCHIVOS
    if (!texturaFondo.loadFromFile("assets/images/menu/fondo_menu.png")) {
        std::cerr << "ERROR: fondo\n";
    }
    if (!texturaConfig.loadFromFile("assets/images/menu/simbolo_configuracion.png")) {
        std::cerr << "ERROR: config.png\n";
    }
    if (!fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "ERROR: fuente\n";
    }

    // 2. CREAR OBJETOS
    spriteFondo = std::make_unique<sf::Sprite>(texturaFondo);
    spriteConfig = std::make_unique<sf::Sprite>(texturaConfig);
    textoJugar = std::make_unique<sf::Text>(fuente);
    textoSalir = std::make_unique<sf::Text>(fuente);

    // 3. CONFIGURAR TODO
    sf::Vector2u sizeFondo = texturaFondo.getSize();
    spriteFondo->setScale({ ancho / (float)sizeFondo.x, alto / (float)sizeFondo.y });

    sf::Vector2u sizeConfig = texturaConfig.getSize();
    spriteConfig->setScale({ 50.f / sizeConfig.x, 50.f / sizeConfig.y });
    spriteConfig->setPosition({ ancho - 70.f, alto - 70.f });

    // Textos
    textoJugar->setString("JUGAR");
    textoJugar->setCharacterSize(35);
    textoJugar->setFillColor(sf::Color::White);

    textoSalir->setString("SALIR");
    textoSalir->setCharacterSize(35);
    textoSalir->setFillColor(sf::Color::White);

    // Botones
    cajaJugar.setSize({300.f, 70.f});
    cajaJugar.setFillColor(sf::Color(200, 0, 0));
    cajaJugar.setOutlineThickness(3);
    cajaJugar.setOutlineColor(sf::Color::White);
    cajaJugar.setPosition({ancho / 2.f - 150.f, alto / 2.f + 50.f});

    sf::FloatRect tj = textoJugar->getLocalBounds();
    textoJugar->setOrigin({ tj.position.x + tj.size.x / 2.f, tj.position.y + tj.size.y / 2.f });
    textoJugar->setPosition({ cajaJugar.getPosition().x + 150.f, cajaJugar.getPosition().y + 35.f });

    cajaSalir.setSize({300.f, 70.f});
    cajaSalir.setFillColor(sf::Color(200, 0, 0));
    cajaSalir.setOutlineThickness(3);
    cajaSalir.setOutlineColor(sf::Color::White);
    cajaSalir.setPosition({ancho / 2.f - 150.f, alto / 2.f + 140.f});

    sf::FloatRect ts = textoSalir->getLocalBounds();
    textoSalir->setOrigin({ ts.position.x + ts.size.x / 2.f, ts.position.y + ts.size.y / 2.f });
    textoSalir->setPosition({ cajaSalir.getPosition().x + 150.f, cajaSalir.getPosition().y + 35.f });

    cajaConfig.setSize({50.f, 50.f});
    cajaConfig.setFillColor(sf::Color::Transparent);
    cajaConfig.setPosition(spriteConfig->getPosition());
}

void Menu::actualizar(sf::Vector2i mousePos) {
    sf::Vector2f m = (sf::Vector2f)mousePos;
    cajaJugar.setFillColor(cajaJugar.getGlobalBounds().contains(m) ? sf::Color(120, 0, 0) : sf::Color(200, 0, 0));
    cajaSalir.setFillColor(cajaSalir.getGlobalBounds().contains(m) ? sf::Color(120, 0, 0) : sf::Color(200, 0, 0));
}

void Menu::dibujar(sf::RenderWindow& ventana) {
    if (spriteFondo) ventana.draw(*spriteFondo);
    
    ventana.draw(cajaJugar);
    if (textoJugar) ventana.draw(*textoJugar);
    
    ventana.draw(cajaSalir);
    if (textoSalir) ventana.draw(*textoSalir);
    
    if (spriteConfig) ventana.draw(*spriteConfig);
}

bool Menu::verificarClickJugar(sf::Vector2i mousePos) {
    return cajaJugar.getGlobalBounds().contains((sf::Vector2f)mousePos);
}

bool Menu::verificarClickSalir(sf::Vector2i mousePos) {
    return cajaSalir.getGlobalBounds().contains((sf::Vector2f)mousePos);
}

bool Menu::verificarClickConfig(sf::Vector2i mousePos) {
    return cajaConfig.getGlobalBounds().contains((sf::Vector2f)mousePos);
}