#pragma once
#include "State.hpp"
#include "Game.hpp"
#include <SFML/Graphics.hpp>
#include "entities/Player.hpp"
#include "Inventory.hpp"
#include "CoordenadasDebug.hpp"
#include "VideoFinalState.hpp"
#include "Obstaculo.hpp"
#include <memory>
#include <vector>
#include <array>
#include <cmath>
#include <random>

// Estructura para las cámaras de vigilancia
struct CamaraVigilancia {
    sf::Vector2f posicion;
    sf::Vector2f direccion;
    float alcance;
    float anguloApertura;         // Ángulo total de visión en grados
    bool esTipoRojo;              // true = roja, false = blanca
    bool activa;
    
    // Obtener los 3 vértices del triángulo de visión
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
        
        return {
            posicion,
            posicion + dirIzq * alcance,
            posicion + dirDer * alcance
        };
    }
};

// Estructura para los minijuegos de desactivación
struct TerminalDesactivacion {
    sf::FloatRect areaInteraccion;
    bool completado;
    std::string textoMensaje;
};

class Centinela3State : public State 
{
private:
    // ========== JUGADOR Y MUNDO ==========
    Player m_player;
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    
    std::vector<Obstaculo> m_mapaFisico;
    void configurarColisiones();
    
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    // ========== CÁMARAS DE VIGILANCIA ==========
    std::vector<CamaraVigilancia> m_camaras;
    void configurarCamaras();
    void dibujarCamaras();
    bool jugadorDetectadoPorCamaras();
    
    // ========== TERMINALES DE DESACTIVACIÓN ==========
    TerminalDesactivacion m_terminalBlancas;
    TerminalDesactivacion m_terminalRojas;
    TerminalDesactivacion m_terminalPuerta;
    void configurarTerminales();
    
    // ========== ESTADO DE PROGRESO ==========
    bool m_camarasBlancasDesactivadas;
    bool m_camarasRojasDesactivadas;
    bool m_puertaAbierta;
    bool m_juegoCompletado;
    
    // ========== MINIJUEGOS ==========
    bool m_minijuegoActivo;
    int m_minijuegoActual;  // 1 = blancas, 2 = rojas, 3 = puerta
    
    // Variables para minijuego 1 y 2 (Secuencia de botones)
    std::vector<int> m_secuenciaBotones;
    int m_indiceSecuencia;
    float m_tiempoMostrandoSecuencia;
    bool m_mostrandoSecuencia;
    float m_tiempoInput;
    
    // Variables para minijuego 3 (Código numérico)
    std::string m_codigoCorrecto;
    std::string m_codigoIngresado;
    
    void iniciarMinijuegoDesactivacion(int tipo);
    void actualizarMinijuegoDesactivacion(float dt);
    void dibujarMinijuegoDesactivacion();
    void completarMinijuego();
    
    void iniciarMinijuegoPuerta();
    void actualizarMinijuegoPuerta(float dt);
    void dibujarMinijuegoPuerta();
    
    // ========== DETECCIÓN ==========
    bool m_jugadorDetectado;
    float m_tiempoDeteccion;
    
    // ========== UI Y FUENTES ==========
    sf::Font m_font;
    bool m_fontLoaded;
    
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoMensaje;
    float m_tiempoMensaje;
    
    // ========== DIÁLOGO DE DECISIÓN FINAL ==========
    bool m_dialogoDecisionActivo;
    int m_opcionSeleccionada;
    std::unique_ptr<sf::Text> m_textoDialogoDecision;
    std::unique_ptr<sf::Text> m_textoOpcion1;
    std::unique_ptr<sf::Text> m_textoOpcion2;
    
    void mostrarDialogoDecision();
    void handleDecisionInput();
    
    // ========== CONTROL ==========
    bool m_debugMode;
    bool m_activo;
    
public:
    Centinela3State(sf::RenderWindow* window, Game* game);
    ~Centinela3State() override = default;
    
    void update(float dt) override;
    void draw() override;
    void handleEvent(const sf::Event& event) override;
};