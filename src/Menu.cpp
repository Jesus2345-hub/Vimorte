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

    // 2. CREAR OBJETOS (Ahora sí llamamos al constructor con los datos listos)
    spriteFondo = std::make_unique<sf::Sprite>(texturaFondo);
    spriteConfig = std::make_unique<sf::Sprite>(texturaConfig);
    textoBoton = std::make_unique<sf::Text>(fuente);
    textoSalir = std::make_unique<sf::Text>(fuente);

    // 3. CONFIGURAR TODO
    sf::Vector2u sizeFondo = texturaFondo.getSize();
    spriteFondo->setScale({ ancho / (float)sizeFondo.x, alto / (float)sizeFondo.y });

    sf::Vector2u sizeConfig = texturaConfig.getSize();
    spriteConfig->setScale({ 50.f / sizeConfig.x, 50.f / sizeConfig.y });
    spriteConfig->setPosition({ ancho - 70.f, alto - 70.f });

    // Textos
    textoBoton->setString("ENTRAR");
    textoBoton->setCharacterSize(35);
    textoBoton->setFillColor(sf::Color::White);

    textoSalir->setString("SALIR");
    textoSalir->setCharacterSize(35);
    textoSalir->setFillColor(sf::Color::White);

    // Botones (Lógica de cajas)
    cajaBoton.setSize({250.f, 70.f});
    cajaBoton.setFillColor(sf::Color(200, 0, 0));
    cajaBoton.setOutlineThickness(3);
    cajaBoton.setOutlineColor(sf::Color::White);
    cajaBoton.setPosition({ancho / 2.f - 125.f, alto / 2.f + 100.f});

    sf::FloatRect tb = textoBoton->getLocalBounds();
    textoBoton->setOrigin({ tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f });
    textoBoton->setPosition({ cajaBoton.getPosition().x + 125.f, cajaBoton.getPosition().y + 35.f });

    cajaSalir.setSize({250.f, 70.f});
    cajaSalir.setFillColor(sf::Color(200, 0, 0));
    cajaSalir.setOutlineThickness(3);
    cajaSalir.setOutlineColor(sf::Color::White);
    cajaSalir.setPosition({ancho / 2.f - 125.f, alto / 2.f + 200.f});

    sf::FloatRect ts = textoSalir->getLocalBounds();
    textoSalir->setOrigin({ ts.position.x + ts.size.x / 2.f, ts.position.y + ts.size.y / 2.f });
    textoSalir->setPosition({ cajaSalir.getPosition().x + 125.f, cajaSalir.getPosition().y + 35.f });

    cajaConfig.setSize({50.f, 50.f});
    cajaConfig.setFillColor(sf::Color::Transparent);
    cajaConfig.setPosition(spriteConfig->getPosition());
}

void Menu::actualizar(sf::Vector2i mousePos) {
    sf::Vector2f m = (sf::Vector2f)mousePos;
    cajaBoton.setFillColor(cajaBoton.getGlobalBounds().contains(m) ? sf::Color(120, 0, 0) : sf::Color(200, 0, 0));
    cajaSalir.setFillColor(cajaSalir.getGlobalBounds().contains(m) ? sf::Color(120, 0, 0) : sf::Color(200, 0, 0));
}

void Menu::dibujar(sf::RenderWindow& ventana) {
    if (spriteFondo) ventana.draw(*spriteFondo);
    ventana.draw(cajaBoton);
    if (textoBoton) ventana.draw(*textoBoton);
    ventana.draw(cajaSalir);
    if (textoSalir) ventana.draw(*textoSalir);
    if (spriteConfig) ventana.draw(*spriteConfig);
}

bool Menu::verificarClick(sf::Vector2i mousePos) {
    return cajaBoton.getGlobalBounds().contains((sf::Vector2f)mousePos);
}

bool Menu::verificarClickSalir(sf::Vector2i mousePos) {
    return cajaSalir.getGlobalBounds().contains((sf::Vector2f)mousePos);
}

bool Menu::verificarClickConfig(sf::Vector2i mousePos) {
    return cajaConfig.getGlobalBounds().contains((sf::Vector2f)mousePos);
}