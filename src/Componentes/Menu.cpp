#include "Componentes/Menu.hpp"
#include <iostream>

Menu::Menu(float ancho, float alto)
{
    if (!texturaFondo.loadFromFile("assets/images/menu/fondo_menu.png")) {
        std::cerr << "ERROR: fondo\n";
    }
    if (!texturaConfig.loadFromFile("assets/images/menu/simbolo_configuracion.png")) {
        std::cerr << "ERROR: config.png\n";
    }
    if (!fuente.openFromFile("assets/fonts/menu/VCR_OSD_MONO.ttf")) {
        std::cerr << "ERROR: fuente\n";
    }

    spriteFondo = std::make_unique<sf::Sprite>(texturaFondo);
    spriteConfig = std::make_unique<sf::Sprite>(texturaConfig);
    textoJugar = std::make_unique<sf::Text>(fuente);
    textoSalir = std::make_unique<sf::Text>(fuente);

    // Fondo escalado para cubrir toda la pantalla
    sf::Vector2u sizeFondo = texturaFondo.getSize();
    float escalaX = ancho / static_cast<float>(sizeFondo.x);
    float escalaY = alto / static_cast<float>(sizeFondo.y);
    float escala = std::max(escalaX, escalaY);
    spriteFondo->setScale(sf::Vector2f(escala, escala));
    spriteFondo->setPosition(sf::Vector2f(ancho/2.f, alto/2.f));
    sf::FloatRect boundsFondo = spriteFondo->getLocalBounds();
    spriteFondo->setOrigin(sf::Vector2f(boundsFondo.size.x/2.f, boundsFondo.size.y/2.f));

    // Config
    spriteConfig->setScale(sf::Vector2f(50.f / texturaConfig.getSize().x, 50.f / texturaConfig.getSize().y));
    spriteConfig->setPosition(sf::Vector2f(ancho - 70.f, alto - 70.f));

    // Botón JUGAR centrado
    textoJugar->setString("JUGAR");
    textoJugar->setCharacterSize(35);
    textoJugar->setFillColor(sf::Color::White);
    
    cajaJugar.setSize(sf::Vector2f(300.f, 70.f));
    cajaJugar.setOrigin(sf::Vector2f(150.f, 35.f));
    cajaJugar.setPosition(sf::Vector2f(ancho/2.f, alto/2.f + 50.f));
    cajaJugar.setFillColor(sf::Color(200, 0, 0));
    cajaJugar.setOutlineThickness(3);
    cajaJugar.setOutlineColor(sf::Color::White);
    
    sf::FloatRect tj = textoJugar->getLocalBounds();
    textoJugar->setOrigin(sf::Vector2f(tj.size.x/2.f, tj.size.y/2.f));
    textoJugar->setPosition(sf::Vector2f(ancho/2.f, alto/2.f + 50.f));

    // Botón SALIR centrado
    textoSalir->setString("SALIR");
    textoSalir->setCharacterSize(35);
    textoSalir->setFillColor(sf::Color::White);
    
    cajaSalir.setSize(sf::Vector2f(300.f, 70.f));
    cajaSalir.setOrigin(sf::Vector2f(150.f, 35.f));
    cajaSalir.setPosition(sf::Vector2f(ancho/2.f, alto/2.f + 140.f));
    cajaSalir.setFillColor(sf::Color(200, 0, 0));
    cajaSalir.setOutlineThickness(3);
    cajaSalir.setOutlineColor(sf::Color::White);
    
    sf::FloatRect ts = textoSalir->getLocalBounds();
    textoSalir->setOrigin(sf::Vector2f(ts.size.x/2.f, ts.size.y/2.f));
    textoSalir->setPosition(sf::Vector2f(ancho/2.f, alto/2.f + 140.f));

    cajaConfig.setSize(sf::Vector2f(50.f, 50.f));
    cajaConfig.setFillColor(sf::Color::Transparent);
    cajaConfig.setPosition(spriteConfig->getPosition());
}

void Menu::redimensionar(float ancho, float alto) {
    // Redimensionar fondo
    sf::Vector2u sizeFondo = texturaFondo.getSize();
    float escalaX = ancho / static_cast<float>(sizeFondo.x);
    float escalaY = alto / static_cast<float>(sizeFondo.y);
    float escala = std::max(escalaX, escalaY); // Usar la mayor para cubrir toda la pantalla
    spriteFondo->setScale(sf::Vector2f(escala, escala));
    
    // Centrar el fondo
    sf::FloatRect boundsFondo = spriteFondo->getLocalBounds();
    spriteFondo->setOrigin(sf::Vector2f(boundsFondo.size.x / 2.f, boundsFondo.size.y / 2.f));
    spriteFondo->setPosition(sf::Vector2f(ancho / 2.f, alto / 2.f));

    // Configurar icono de configuración
    float configSize = 50.f;
    sf::Vector2u sizeConfig = texturaConfig.getSize();
    float escalaConfigX = configSize / static_cast<float>(sizeConfig.x);
    float escalaConfigY = configSize / static_cast<float>(sizeConfig.y);
    spriteConfig->setScale(sf::Vector2f(escalaConfigX, escalaConfigY));
    spriteConfig->setPosition(sf::Vector2f(ancho - 70.f, alto - 70.f));

    // Configurar textos
    textoJugar->setString("JUGAR");
    textoJugar->setCharacterSize(35);
    textoJugar->setFillColor(sf::Color::White);

    textoSalir->setString("SALIR");
    textoSalir->setCharacterSize(35);
    textoSalir->setFillColor(sf::Color::White);

    // Botón JUGAR
    cajaJugar.setSize(sf::Vector2f(300.f, 70.f));
    cajaJugar.setFillColor(sf::Color(200, 0, 0));
    cajaJugar.setOutlineThickness(3);
    cajaJugar.setOutlineColor(sf::Color::White);
    cajaJugar.setOrigin(sf::Vector2f(150.f, 35.f)); // Centro del botón
    cajaJugar.setPosition(sf::Vector2f(ancho / 2.f, alto / 2.f + 50.f));

    sf::FloatRect tj = textoJugar->getLocalBounds();
    textoJugar->setOrigin(sf::Vector2f(tj.size.x / 2.f, tj.size.y / 2.f));
    textoJugar->setPosition(sf::Vector2f(ancho / 2.f, alto / 2.f + 50.f));

    // Botón SALIR
    cajaSalir.setSize(sf::Vector2f(300.f, 70.f));
    cajaSalir.setFillColor(sf::Color(200, 0, 0));
    cajaSalir.setOutlineThickness(3);
    cajaSalir.setOutlineColor(sf::Color::White);
    cajaSalir.setOrigin(sf::Vector2f(150.f, 35.f)); // Centro del botón
    cajaSalir.setPosition(sf::Vector2f(ancho / 2.f, alto / 2.f + 140.f));

    sf::FloatRect ts = textoSalir->getLocalBounds();
    textoSalir->setOrigin(sf::Vector2f(ts.size.x / 2.f, ts.size.y / 2.f));
    textoSalir->setPosition(sf::Vector2f(ancho / 2.f, alto / 2.f + 140.f));

    cajaConfig.setSize(sf::Vector2f(50.f, 50.f));
    cajaConfig.setFillColor(sf::Color::Transparent);
    cajaConfig.setPosition(spriteConfig->getPosition());
}

void Menu::actualizar(sf::Vector2i mousePos) {
    sf::Vector2f m = static_cast<sf::Vector2f>(mousePos);
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
    return cajaJugar.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

bool Menu::verificarClickSalir(sf::Vector2i mousePos) {
    return cajaSalir.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

bool Menu::verificarClickConfig(sf::Vector2i mousePos) {
    return cajaConfig.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}