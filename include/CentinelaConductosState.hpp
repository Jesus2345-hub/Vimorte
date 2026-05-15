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
    // Cámara
    sf::View m_camera;
    sf::Vector2f m_worldSize;

    // Sistema de humo tóxico
    struct HumoToxico
    {
        sf::Vector2f posicion;
        float cicloTimer = 0.f;
        bool activo = false;
        float duracionActivo = 3.0f;
        float duracionApagado = 2.5f;
        sf::FloatRect areaColision;
        float anchoColision = 213.f;
        float altoColision = 443.f;
        float escalaBase = 1.0f;
        float escalas[5] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
        float rotacion = 0.f;
        // Visual
        std::vector<sf::Texture> texturas; // 0=aparece, 1=crece, 2/3=activo alterna, 4=desaparece
        std::unique_ptr<sf::Sprite> sprite;
        int frameActual = -1; // -1 = sin imagen
        float frameTimer = 0.f;
        float frameDuration = 0.15f;

        // Fases del humo
        enum class Fase
        {
            APAGADO,
            APARECIENDO,
            ACTIVO,
            DESAPARECIENDO
        };
        Fase fase = Fase::APAGADO;
        float faseTimer = 0.f;
    };

    std::vector<HumoToxico> m_humos;
    void inicializarHumos();
    void actualizarHumos(float dt);
    void dibujarHumos(sf::RenderWindow &window);
    bool verificarColisionHumos();

    // Salida con rejilla
    sf::Texture m_rejillaSalidaTexture;
    std::unique_ptr<sf::Sprite> m_rejillaSalidaSprite;
    bool m_cercaSalida = false;

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

    bool m_debugMode = false;

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