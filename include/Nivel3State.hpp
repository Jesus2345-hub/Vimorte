#ifndef NIVEL3STATE_HPP
#define NIVEL3STATE_HPP

#include "State.hpp"
#include "entities/Player.hpp"
#include "MinigameCables.hpp"
#include "MinigamePatron.hpp"
#include "MinigameMemoria.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Nivel3State : public State {
public:
    Nivel3State(sf::RenderWindow* window, Game* game);
    ~Nivel3State() override = default;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw() override;

private:
    Player m_player;
    sf::Texture m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    sf::Vector2f m_worldSize;
    sf::View m_camera;
    sf::Vector2u m_lastWindowSize;
    
    // Colisiones del mapa
    std::vector<sf::FloatRect> m_mapaFisico;
    void configurarColisiones();
    
    // Áreas de interacción
    sf::FloatRect m_bombaArea;
    sf::FloatRect m_pista1Area;
    sf::FloatRect m_pista2Area;
    sf::FloatRect m_puertaSalidaArea;
    
    // Estados de cercanía
    bool m_cercaBomba = false;
    bool m_cercaPista1 = false;
    bool m_cercaPista2 = false;
    bool m_cercaPuertaSalida = false;
    
    // Sistema de bomba
    float m_tiempoRestante = 180.0f;
    bool m_bombaExploto = false;
    bool m_bombaDesactivada = false;
    int m_cablesCorrectos = 0;
    const int m_totalCables = 2;
    
    // Cronómetro visual
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInteraccion;
    std::unique_ptr<sf::Text> m_textoTiempo;
    std::unique_ptr<sf::Text> m_textoBomba;
    std::unique_ptr<sf::Text> m_textoMensaje;
    
    // Control de tutorial
    bool m_mostrarTutorial = false;
    bool m_mostrarTutorialPorTecla = false;
    bool m_escapeConsumed = false;
    
    // ===== AÑADIR AQUÍ: Sistema de pistas =====
    std::string m_pista1Texto = "";
    std::string m_pista2Texto = "";
    bool m_pista1Encontrada = false;
    bool m_pista2Encontrada = false;
    std::vector<int> m_ordenCorrectoBomba;  // Orden correcto para desactivar
    std::string nombreColor(int indice) const;  // Helper para nombres de colores
    // =========================================
    
    // Funciones helper
    void verificarSalidaNivel();
    void mostrarMensaje(const std::string& texto, float duracion = 2.0f, sf::Color color = sf::Color::Yellow);
    void actualizarTextosMinijuegos();
    void actualizarCronometro(float dt);
    void explotarBomba();
    void cortarCable(int numeroCable);
    sf::Color getColorTiempo() const;
    
    struct MensajeTemporal {
        std::string texto;
        float tiempoRestante = 0.0f;
        sf::Color color = sf::Color::Yellow;
    };
    MensajeTemporal m_msjActual;
    
    MinigameCables m_minijuegoCables;
    
std::string generarTextoPista(const std::vector<int>& orden, 
                              int r1, int r2, int r3 = -1, int r4 = -1) const;
                              
 


MinigamePatron m_minijuegoPatron;

MinigameMemoria m_minijuegoMemoria;
};

#endif