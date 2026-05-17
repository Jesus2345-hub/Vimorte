// CentinelaGameOverState.hpp
#pragma once
#include "Estados/State.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class CentinelaGameOverState : public State {
private:
    bool m_modoSupervivencia;
    std::string m_nivelOriginalId;
    bool m_finalMostrado;
    
    // Elementos UI
    sf::Font m_fuente;
    sf::RectangleShape m_fondo;
    sf::RectangleShape m_panel;
    
    // Títulos
    std::unique_ptr<sf::Text> m_titulo;
    std::unique_ptr<sf::Text> m_tituloSombra;
    std::unique_ptr<sf::Text> m_subtitulo;
    
    // Botones (varían según el modo)
    std::unique_ptr<sf::Text> m_reintentarCentinelaText;
    sf::RectangleShape m_botonReintentarCentinela;
    bool m_hoverReintentar = false;
    
    // NUEVO: Botón para aceptar final malo (solo modo historia)
    std::unique_ptr<sf::Text> m_aceptarFinalMaloText;
    sf::RectangleShape m_botonAceptarFinalMalo;
    bool m_hoverAceptarFinalMalo = false;
    
    // Botones comunes
    std::unique_ptr<sf::Text> m_regresarNivelText;
    sf::RectangleShape m_botonRegresarNivel;
    bool m_hoverRegresar = false;
    
    std::unique_ptr<sf::Text> m_menuText;
    sf::RectangleShape m_botonMenu;
    bool m_hoverMenu = false;
    
    float m_tiempoAnimacion;
    
    // Métodos de acción
    void reiniciarCentinela();
    void aceptarFinalMalo();    
    void regresarANivel();
    void irMenuPrincipal();
    
public:
    CentinelaGameOverState(sf::RenderWindow* window, Game* game, 
                           bool modoSupervivencia, 
                           const std::string& nivelOriginalId = "");
    
    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;
    void draw() override;
};