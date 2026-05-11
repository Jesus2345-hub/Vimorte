#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <random>

struct Pregunta {
    std::string enunciado;
    std::vector<std::string> opciones;      // Opciones de texto
    std::vector<std::string> imagenes;      // Rutas de imágenes (vacío = pregunta de texto)
    bool esPreguntaConImagenes;             // true = usa imágenes, false = usa texto
    int respuestaCorrecta;                  // Índice 0-3 (o 0-2 para imágenes)
};

class MinigameQuiz {
private:
    // Ventana y UI
    sf::RectangleShape m_background;
    sf::Texture m_pizarraTexture;
    std::unique_ptr<sf::Sprite> m_pizarraSprite;
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_tituloText;
    std::unique_ptr<sf::Text> m_preguntaText;
    std::unique_ptr<sf::Text> m_puntuacionText;
    std::unique_ptr<sf::Text> m_instruccionesText;
    std::unique_ptr<sf::Text> m_resultadoText;
    std::vector<bool> m_tieneImagen;

    // Opciones de TEXTO
    std::vector<std::unique_ptr<sf::Text>> m_textosBotones;
    
    // Opciones de IMAGEN
    std::vector<std::unique_ptr<sf::Sprite>> m_imagenesBotones;
    std::vector<sf::Texture> m_texturasBotones;
    std::vector<std::unique_ptr<sf::RectangleShape>> m_bordesHover;  // Borde amarillo para hover
    
    // Hover
    std::vector<bool> m_botonHover;
    
    // Banco de preguntas
    std::vector<Pregunta> m_bancoPreguntas;
    std::vector<int> m_preguntasDisponibles;
    Pregunta* m_preguntaActual;
    
    // Estado del juego
    bool m_isActive;
    bool m_juegoTerminado;
    int m_preguntasRespondidas;
    int m_respuestasCorrectas;
    int m_preguntasPorRonda;
    int m_opcionSeleccionada;
    bool m_mostrandoResultado;
    float m_tiempoResultado;
    std::string m_mensajeResultado;
    sf::Color m_colorResultado;
    
    // Random
    std::random_device m_rd;
    std::mt19937 m_gen;
    
    // Dimensiones
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    
public:
    MinigameQuiz();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void reset();
    bool isCompleted() const { return m_juegoTerminado && m_respuestasCorrectas >= m_preguntasPorRonda; }
    
private:
    void inicializarBancoPreguntas();
    void seleccionarSiguientePregunta();
    void verificarRespuesta(int opcion);
    void configurarUI();
    void actualizarTextos();
    void actualizarImagenes();
    void ocultarOpcionesTexto();
    void ocultarOpcionesImagen();
    void mostrarResultadoTemporal(const std::string& mensaje, const sf::Color& color);
};