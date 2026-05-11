#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp"
#include "Inventory.hpp"
#include "entities/Balon.hpp"
#include "entities/LeBron.hpp"
#include "MinigameBaloncesto.hpp"
#include "CentinelaConductosState.hpp"
#include <memory>
#include <vector>

class Nivel7State : public State
{
private:
    Player m_player;

    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;

    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();

    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;

    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_fontLoaded;

    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    struct MensajeTemporal
    {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;
    void mostrarMensaje(const std::string &texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);

    // Sprites para objetos en el mapa
    sf::Texture m_balonBasketMapTexture;
    std::unique_ptr<sf::Sprite> m_balonBasketMapSprite;
    sf::Texture m_destornilladorMapTexture;
    std::unique_ptr<sf::Sprite> m_destornilladorMapSprite;

    // Balones
    std::vector<std::unique_ptr<Balon>> m_balones;
    std::vector<bool> m_balonEnArco;
    sf::FloatRect m_arcoArea;
    int m_goles;
    int m_golesParaGanar;

    // Destornillador
    bool m_tieneDestornillador;
    sf::FloatRect m_destornilladorArea;
    bool m_cercaDestornillador;

    // Entrada centinela
    sf::FloatRect m_entradaCentinelaArea;
    bool m_cercaEntradaCentinela;
    sf::Texture m_rejillaTexture;
    std::unique_ptr<sf::Sprite> m_rejillaSprite;

    // Baloncesto
    LeBron m_lebron;
    MinigameBaloncesto m_baloncestoMinigame;
    bool m_tieneBalonBasket;
    sf::FloatRect m_balonBasketArea;
    bool m_cercaBalonBasket;
    bool m_cercaLebron;
    bool m_llaveObtenida;

    // Salida
    sf::FloatRect m_puertaSalidaArea;
    bool m_cercaPuertaSalida;
    void verificarSalidaNivel();

public:
    Nivel7State(sf::RenderWindow *window, Game *game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event &event) override;
    ~Nivel7State() override = default;
};