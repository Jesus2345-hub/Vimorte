#pragma once
#include "Estados/State.hpp"
#include "Configuracion/Game.hpp"
#include <SFML/Graphics.hpp>
#include "Entidades/Player.hpp"
#include "Entidades/Obstaculo.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include <memory>
#include <vector>
#include <optional>

class LobbyState : public State
{
private:
    Player m_player;

    // Fondo del lobby
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    bool m_debugMode = false;

    // Lógica de colisiones
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();

    // Área del ascensor
    sf::FloatRect m_ascensorArea;
    bool m_cercaAscensor;

    // UI de interacción
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;

    // Control de guardado
    bool m_guardadoRealizado = false;

    sf::View m_camera;
    sf::Vector2f m_worldSize;

public:
    LobbyState(sf::RenderWindow *window, Game *game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~LobbyState() override = default;
};