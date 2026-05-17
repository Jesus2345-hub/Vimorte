#pragma once
#include "Estados/State.hpp"
#include "Configuracion/Game.hpp"
#include <SFML/Graphics.hpp>
#include "Entidades/Player.hpp"
#include "Componentes/Inventory.hpp"
#include "Configuracion/CoordenadasDebug.hpp"
#include "Estados/VideoFinalState.hpp"
#include "Entidades/Obstaculo.hpp"
#include <memory>
#include <vector>
#include <array>
#include <cmath>
#include <random>

// Estructura que guarda los datos de una camara de vigilancia
struct CamaraVigilancia {
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float alcance;
    float anguloApertura;
    bool esTipoRojo;
    bool activa;
    
    // Calcula los 3 puntos del triangulo de vision de la camara
    std::array<sf::Vector2f, 3> obtenerTriangulo() const {
        float mitadAngulo = anguloApertura / 2.0f;
        float rad = mitadAngulo * 3.14159f / 180.0f;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);
        
        sf::Vector2f dirIzq(
            direccion.x * cosA - direccion.y * sinA,
            direccion.x * sinA + direccion.y * cosA
        );
        sf::Vector2f dirDer(
            direccion.x * cosA + direccion.y * sinA,
            -direccion.x * sinA + direccion.y * cosA
        );
        
        return {posicion, posicion + dirIzq * alcance, posicion + dirDer * alcance};
    }
};

// Estructura para las terminales donde se activan los minijuegos
struct TerminalDesactivacion {
    sf::FloatRect areaInteraccion;
    bool completado;
    std::string textoMensaje;
};

class Centinela3State : public State 
{
private:
    // JUGADOR Y MUNDO
    Player m_player;
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    sf::View m_camera;
    
    // CAMARAS DE VIGILANCIA
    std::vector<CamaraVigilancia> m_camaras;
    void configurarCamaras();
    void dibujarCamaras();
    bool jugadorDetectadoPorCamaras();
    
    // TERMINALES Y PUERTA DE SALIDA
    TerminalDesactivacion m_terminalBlancas;
    TerminalDesactivacion m_terminalRojas;
    sf::FloatRect m_areaPuerta;
    void configurarTerminales();
    
    // ESTADO DE PROGRESO DEL NIVEL
    bool m_camarasBlancasDesactivadas;
    bool m_camarasRojasDesactivadas;
    bool m_puertaAbierta;
    bool m_juegoCompletado;
    
    // MINIJUEGOS
    bool m_minijuegoActivo;
    int m_minijuegoActual;  // 1 = hackeo blancas, 2 = interruptores rojas
    
    // Minijuego 1: Hackeo de terminal (adivinar palabra)
    std::string m_palabraObjetivo;
    std::string m_palabraIngresada;
    int m_letraActual;
    float m_tiempoHackeo;
    std::vector<std::string> m_palabrasDisponibles;
    float m_letrasCayendoOffset;
    
    // Minijuego 2: Panel de interruptores (igualar patron)
    std::vector<bool> m_interruptores;
    std::vector<bool> m_patronObjetivo;
    int m_interruptorSeleccionado;
    
    // CRONOMETRO Y EFECTO DE ALARMA
    float m_tiempoLimite;
    float m_tiempoAlarma;
    float m_intensidadAlarma;
    
    // PANEL DE AYUDA INICIAL
    bool m_panelAyudaActivo;
    
    void iniciarMinijuegoHackeo();
    void actualizarMinijuegoHackeo(float dt);
    void dibujarMinijuegoHackeo();
    
    void iniciarMinijuegoInterruptores();
    void actualizarMinijuegoInterruptores(float dt);
    void dibujarMinijuegoInterruptores();
    
    void completarMinijuego();
    
    // DETECCION POR CAMARAS
    bool m_jugadorDetectado;
    float m_tiempoDeteccion;
    
    // FUENTE Y TEXTOS DE INTERFAZ
    sf::Font m_font;
    bool m_fontLoaded;
    
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    float m_tiempoMensaje;
    
    // DIALOGO DE DECISION FINAL
    bool m_dialogoDecisionActivo;
    int m_opcionSeleccionada;
    std::unique_ptr<sf::Text> m_textoDialogoDecision;
    std::unique_ptr<sf::Text> m_textoOpcion1;
    std::unique_ptr<sf::Text> m_textoOpcion2;
    
    void mostrarDialogoDecision();
    void manejarDecisionInput();
    
    // CONTROL GENERAL
    bool m_debugMode;
    bool m_activo;
    
public:
    Centinela3State(sf::RenderWindow* window, Game* game);
    ~Centinela3State() override = default;
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};