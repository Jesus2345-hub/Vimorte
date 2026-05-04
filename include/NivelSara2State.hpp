#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "CriminalCaseMinigame.hpp"
#include "Obstaculo.hpp"
#include "Inventory.hpp"
#include <memory>
#include <vector>

struct BloqueInteractivo {
    sf::FloatRect area;
    std::string mensaje;
};
  

class NivelSara2State : public State 
{
private:
    //Juego de Crimen
    CriminalCaseMinigame m_criminalMinigame;
    sf::FloatRect m_criminalArea;
    bool m_cercaCriminalArea;
    bool m_criminalGameCompleted;

    void configurarMinijuegoCriminal();

    Player m_player;
    
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    sf::View m_camera;
    
    // ========== SISTEMA DE MENSAJES TEMPORALES ==========
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    };
    MensajeTemporal m_msjActual;
    std::unique_ptr<sf::Text> m_textoMensaje;
    
    bool m_fontLoaded; 
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla; 
    bool m_escapeConsumed;
    
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    
    // === Para conexión con árbol ===
    bool m_mostrarPuertaSalida;
    bool m_cercaPuertaSalida;
    sf::FloatRect m_puertaSalidaArea;
    
    void verificarSalidaNivel();
    void verificarEntradaCentinela();
    void jugadorHaMuerto();
    sf::Vector2u m_lastWindowSize;
    
    std::vector<BloqueInteractivo> m_bloquesInteractivos;
    bool m_cercaBloqueInteractivo;
    bool m_mensajeEmergenteActivo;  
    int m_bloqueActualIndex;
    void configurarBloquesInteractivos();
    void reajustarMinijuegoCriminal();
    std::vector<ObjetoBuscar> m_objetosCriminal;
    std::vector<Sospechoso> m_sospechososCriminal;

public:
    NivelSara2State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~NivelSara2State() override = default;
};