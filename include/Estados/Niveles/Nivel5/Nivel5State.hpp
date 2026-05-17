#pragma once
#include "Estados/State.hpp"
#include "Configuracion/Game.hpp"
#include <SFML/Graphics.hpp>
#include "Entidades/Player.hpp"
#include "Estados/Niveles/Nivel5/CriminalCaseMinigame.hpp"
#include "Entidades/Obstaculo.hpp"
#include "Componentes/Inventory.hpp"
#include <memory>
#include <iostream>
#include <vector>

struct BloqueInteractivo {
    sf::FloatRect area;
    std::string mensaje;
};
  

class Nivel5State : public State 
{
private:
    bool m_estrellaUsada;
    sf::Vector2f m_playerStartPosition = sf::Vector2f(0.f, 0.f);
    std::unique_ptr<sf::Text> m_mensajeFlotante;
    sf::Clock m_clockFlotante;
    float m_tiempoFlotante;
    
    bool m_casoResuelto;
    bool m_nivelCompletado;
    int m_setActualCaso;
    void mostrarMensajeFlotante(const std::string& texto, float duracion = 3.0f, sf::Color color = sf::Color::Red);
    void reajustarMinijuegoCriminalManteniendoEstado();
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
    std::vector<std::vector<ObjetoBuscar>> m_todosLosObjetos;
    std::vector<std::vector<Sospechoso>> m_todosLosSospechosos;
    std::vector<std::vector<DialogoNarrativo>> m_todosLosDialogos;
public:
    Nivel5State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    void setPlayerStartPosition(const sf::Vector2f& pos) { m_playerStartPosition = pos; }
    sf::Vector2f getPlayerStartPosition() const { return m_playerStartPosition; }
    ~Nivel5State() override = default;
};