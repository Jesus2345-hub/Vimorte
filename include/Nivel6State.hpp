#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp"
#include "Inventory.hpp"
#include "entities/Gallo.hpp"
#include <memory>
#include <vector>
#include "MinigameRoosterHunt.hpp"
#include "MinigameTicTacToe.hpp"
#include "entities/Joven.hpp"
#include "entities/Abuelita.hpp"

class Nivel6State : public State 
{
private:
    Player m_player;
    
    // Fondo del nivel 6
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;

    // Sprites para objetos en el mapa
    sf::Texture m_rifleMapTexture;
    std::unique_ptr<sf::Sprite> m_rifleMapSprite;

    // Minijuego RoosterHunt
    MinigameRoosterHunt m_roosterHuntMinigame;

    // Tic Tac Toe
    MinigameTicTacToe m_tictactoeMinigame;
    bool m_gallinaObtenida;
    bool m_dientesObtenidos;

    // Objeto recogible (rifle)
    bool m_rifleRecogido;                        
    sf::FloatRect m_rifleArea;                   
    bool m_cercaRifle;

    // Gallo del gallinero
    Gallo m_gallo;
    sf::FloatRect m_galloArea;
    bool m_cercaGallo;

    // Joven dormido
    Joven m_joven;              
    sf::FloatRect m_jovenArea;
    bool m_cercaJoven;   

    // Abuelita
    Abuelita m_abuelita;
    sf::FloatRect m_abuelitaArea;
    bool m_cercaAbuelita;
    bool m_llaveObtenida;

    // Sistema de colisiones
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    // Cámara
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    // Tutorial
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla;
    bool m_escapeConsumed;
    bool m_fontLoaded;
    
    // Fuente y textos
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    } m_msjActual;
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    
    // Áreas de interacción
    sf::FloatRect m_puertaSalidaArea;
    bool m_cercaPuertaSalida;
    bool m_mostrarPuertaSalida;

    sf::Clock m_tiempoUltimaR;
    bool m_bloquearR;
    
    // Métodos
    void verificarSalidaNivel();
    void verificarEntradaCentinela();
    void jugadorHaMuerto();
    
public:
    Nivel6State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~Nivel6State() override = default;
};