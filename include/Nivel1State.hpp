#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Obstaculo.hpp"
#include "MinigameColorMix.hpp"
#include "MinigamePool.hpp"
#include "MinigameQuiz.hpp"
#include "Inventory.hpp"
#include <memory>
#include <vector>


class Nivel1State : public State 
{
private:
    Player m_player;
    
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    sf::View m_camera;
    
    MinigamePool m_poolMinigame;
    MinigameQuiz m_quizMinigame;
    MinigameColorMix m_colorMixMinigame;
    sf::FloatRect m_pizarraArea;    
    sf::FloatRect m_mesaColorMixArea;
    sf::FloatRect m_mesaPoolArea; 
    
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
    
    bool m_cercaMesaPool;
    bool m_cercaPizarra;
    bool m_cercaMesaColorMix; 
    bool m_mostrarTutorial;
    bool m_mostrarTutorialPorTecla; 
    bool m_escapeConsumed;
    
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    
    // === NUEVO: Para conexión con árbol ===
    bool m_mostrarPuertaSalida;
    bool m_cercaPuertaSalida;
    sf::FloatRect m_puertaSalidaArea;
    
    sf::Clock m_tiempoUltimaE;
    bool m_esperandoSegundaE;
    
    void verificarSalidaNivel();
    void verificarEntradaCentinela();
    void jugadorHaMuerto();
    
public:
    Nivel1State(sf::RenderWindow* window, Game* game);
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
    ~Nivel1State() override = default;
};