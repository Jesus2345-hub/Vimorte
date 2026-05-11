#pragma once
#include "State.hpp"
#include "Game.hpp"
#include "entities/Player.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class CentinelaConductosState : public State
{
private:
    // Jugador
    Player m_player;

    // Fondo
    sf::RectangleShape m_background;

    // Fondo
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_backgroundSprite;

    // Laberinto (paredes)
    std::vector<sf::RectangleShape> m_walls;
    std::vector<sf::FloatRect> m_wallBounds;

    // Salida
    sf::RectangleShape m_exitZone;
    sf::FloatRect m_exitBounds;

    // Cronómetro
    float m_tiempoRestante;
    const float m_tiempoMaximo = 60.0f;
    bool m_tiempoAgotado;
    bool m_nivelCompletado;

    // Fuente y textos
    sf::Font m_font;
    bool m_fontLoaded;
    std::unique_ptr<sf::Text> m_tituloText;
    std::unique_ptr<sf::Text> m_cronometroText;
    std::unique_ptr<sf::Text> m_instruccionesText;
    std::unique_ptr<sf::Text> m_mensajeText;

    // Mensaje temporal
    float m_mensajeTimer;

    // Efecto de parpadeo cuando queda poco tiempo
    float m_parpadeoTimer;
    bool m_parpadeoVisible;

    void configurarLaberinto();
    void verificarSalida();
    void actualizarCronometro(float dt);
    void mostrarMensaje(const std::string &texto, float duracion);

public:
    CentinelaConductosState(sf::RenderWindow *window, Game *game);
    ~CentinelaConductosState() override = default;

    void handleEvent(const sf::Event &event) override;
    void update(float dt) override;
    void draw() override;
};