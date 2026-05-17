#ifndef MINIGAMEMEMORIA_HPP
#define MINIGAMEMEMORIA_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>
#include <random>

class MinigameMemoria {
public:
    MinigameMemoria();
    
    void setSize(const sf::Vector2f& size);
    void setPosition(const sf::Vector2f& position);
    void activate();
    void deactivate();
    bool isActive() const { return m_active; }
    bool isCompleted() const { return m_completed; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }

private:
    bool m_active = false;
    bool m_completed = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // 4 botones como el reactor
    struct BotonReactor {
        sf::RectangleShape forma;
        sf::Color colorNormal;
        sf::Color colorBrillante;
        bool iluminado = false;
        float tiempoIluminado = 0.0f;
        int indice;
    };
    std::vector<BotonReactor> m_botones;
    
    // Secuencia
    std::vector<int> m_secuencia;         // Secuencia a memorizar
    std::vector<int> m_secuenciaJugador;  // Lo que el jugador ha ingresado
    int m_pasoActual = 0;
    int m_rondaActual = 0;
    const int m_rondasMaximas = 4;        // 4 rondas para ganar
    int m_longitudSecuencia = 3;          // Empieza con 3, sube cada ronda
    
    // Estados
    enum class Estado {
        MOSTRANDO_SECUENCIA,  // La máquina muestra la secuencia
        ESPERANDO_JUGADOR,    // El jugador debe repetir
        CORRECTO,             // Acierto momentáneo
        ERROR                 // Fallo momentáneo
    };
    Estado m_estado = Estado::MOSTRANDO_SECUENCIA;
    
    int m_indiceMostrando = 0;       // Índice que se está mostrando
    float m_tiempoEntreBotones = 0.0f;
    const float m_intervaloBotones = 0.9f;  // Tiempo entre cada botón
    const float m_duracionIluminado = 0.6f; // Cuánto brilla cada botón
    
    float m_tiempoMensaje = 0.0f;
    
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInstrucciones;
    std::unique_ptr<sf::Text> m_textoEstado;
    std::unique_ptr<sf::Text> m_textoRonda;
    
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    
    sf::Texture m_texturaFondo;
    std::unique_ptr<sf::Sprite> m_spriteFondo;
    bool m_fondoCargado = false;
    
    std::function<void()> m_onComplete;
    std::mt19937 m_gen;
    
    void inicializarBotones();
    void generarSecuencia();
    void mostrarSiguienteBoton();
    void botonPresionado(int indice);
    void avanzarRonda();
    void reiniciar();
    sf::Color obtenerColorBoton(int indice) const;
};

#endif