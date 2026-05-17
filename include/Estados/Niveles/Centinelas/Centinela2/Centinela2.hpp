#pragma once
#include "Estados/State.hpp"
#include "Configuracion/Game.hpp"
#include <SFML/Graphics.hpp>
#include "Entidades/Player.hpp"
#include "Estados/Niveles/Centinelas/Centinela2/MiniGameCook.hpp"
#include "Entidades/Obstaculo.hpp"
#include "Componentes/Inventory.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include <memory>
#include <vector>

// Estructura para bloque interactivo (ascensor)
struct BloqueInteractivoCentinela {
    sf::FloatRect area;
    std::string mensaje;
};

class Centinela2State : public State 
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
    
    // ========== MINIJUEGO DE COCINA ==========
    std::unique_ptr<CocinaMinigameState> m_cocinaMinigame;
    
    // Áreas del minijuego
    sf::FloatRect m_areaCocina;
    sf::FloatRect m_areaEntrega;
    sf::FloatRect m_areaMenuPlato;
    sf::FloatRect m_areaEstanteCarnes;
    sf::FloatRect m_areaEstanteVerduras;
    sf::FloatRect m_areaEstanteOtros;
    sf::FloatRect m_centinelaArea;
    
    void configurarAreasCocina();
    
    // ========== TIMER Y PROGRESO ==========
    float m_tiempoRestante;
    int m_platosEntregados;
    int m_platosRequeridos;
    bool m_juegoActivo;
    bool m_gameOver;
    int m_muertes;
    
    void actualizarTimer(float dt);
    void verificarEntregaPlato();
    void actualizarPlatoRequerido();
    
    // ========== UI ==========
    std::unique_ptr<sf::Text> m_textoTimer;
    std::unique_ptr<sf::Text> m_textoContador;
    std::unique_ptr<sf::Text> m_textoPlatoActual;
    sf::RectangleShape m_timerBarra;
    
    // ========== SISTEMA DE MENSAJES ==========
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
    };
    MensajeTemporal m_msjActual;
    std::unique_ptr<sf::Text> m_textoMensaje;
    
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    
    // ========== MENSAJE FLOTANTE CENTRADO (con fondo) ==========
    std::unique_ptr<sf::Text> m_mensajeFlotante;
    float m_tiempoFlotante;
    sf::Clock m_clockFlotante;
    void mostrarMensajeFlotante(const std::string& texto, float duracion = 3.0f, sf::Color color = sf::Color::Yellow);    
    // ========== INTERACCIÓN ==========
    bool m_cercaCocina;
    bool m_cercaEntrega;
    bool m_cercaMenu;
    bool m_cercaCentinela;
    std::string m_estanteCerca;
    
    // ========== UI TEXTOS INTERACTIVOS ==========
    sf::Font m_font;
    bool m_fontLoaded;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    
    // ========== CONTROL ==========
    bool m_debugMode;
    bool m_activo;
    bool m_nivelCompletado;
    
    // ========== BLOQUE INTERACTIVO ASCENSOR ==========
    std::vector<BloqueInteractivoCentinela> m_bloquesInteractivos;
    bool m_cercaBloqueInteractivo;
    bool m_mensajeEmergenteActivo;
    int m_bloqueActualIndex;
    void configurarBloqueAscensor();
    
    // ========== DECISIÓN FINAL ==========
    bool m_dialogoDecisionActivo;
    int m_opcionSeleccionada;
    std::unique_ptr<sf::Text> m_textoDialogoDecision;
    std::unique_ptr<sf::Text> m_textoOpcion1;
    std::unique_ptr<sf::Text> m_textoOpcion2;
    
    void mostrarDialogoDecision();
    void handleDecisionInput();
    float m_tiempoBloqueoAscensor;
    bool m_estanteAbierto;
    std::string m_mensajeEstante;
    std::string m_estanteActualNombre;
    std::vector<Ingrediente> m_ingredientesEstanteActual;
    
public:
    Centinela2State(sf::RenderWindow* window, Game* game);
    ~Centinela2State() override = default;
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};