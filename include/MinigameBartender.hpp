#ifndef MINIGAMEBARTENDER_HPP
#define MINIGAMEBARTENDER_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <random>

class MinigameBartender {
public:
    MinigameBartender();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void setDineroJugador(int* dinero) { m_dineroJugador = dinero; }
    int* getDineroJugador() { return m_dineroJugador; }
    
    int getBebidasServidas() const { return m_bebidasServidas; }
    int getErrores() const { return m_errores; }
    bool isGameOver() const { return m_errores >= m_maxErrores; }

private:
    bool m_isActive = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // Dinero
    int* m_dineroJugador = nullptr;
    
    // Progreso
    int m_bebidasServidas = 0;
    int m_errores = 0;
    int m_maxErrores = 3;
    
    // Colores
    enum class ColorBebida {
        ROJO, AZUL, AMARILLO, BLANCO,
        NARANJA, MORADO, VERDE, ROSA, CELESTE, CREMA
    };
    
    std::vector<std::string> m_nombresColores;
    std::vector<sf::Color> m_coloresSFML;
    
    // Pedido actual
    ColorBebida m_pedidoActual;
    sf::Color m_colorPedido;
    std::string m_nombrePedido;
    
    // Mezcla actual
    bool m_tienePrimerColor = false;
    ColorBebida m_primerColor;
    sf::Color m_colorMezcla;
    
    // Fondos
    sf::RectangleShape m_background;
    sf::Texture m_fondoTexture;
    std::unique_ptr<sf::Sprite> m_fondoSprite;
    
    // Copa
    sf::Texture m_copaTexture;
    std::unique_ptr<sf::Sprite> m_copaSprite;
    
    // Botellas
    struct Botella {
        sf::RectangleShape boton;
        std::unique_ptr<sf::Text> texto;
        sf::Color color;
        ColorBebida tipo;
        bool hover = false;
    };
    std::vector<Botella> m_botellas;
    
    // Robot (texto por ahora)
    std::unique_ptr<sf::Text> m_textoRobot;
    std::unique_ptr<sf::Text> m_textoPedido;
    std::unique_ptr<sf::Text> m_textoInstrucciones;
    
    // Fuente
    sf::Font m_font;
    bool m_fontLoaded = false;
    
    // Textos UI
    std::unique_ptr<sf::Text> m_tituloText;
    std::unique_ptr<sf::Text> m_dineroText;
    std::unique_ptr<sf::Text> m_bebidasText;
    std::unique_ptr<sf::Text> m_erroresText;
    std::unique_ptr<sf::Text> m_resultadoText;
    
    // Mensaje
    bool m_mostrandoResultado = false;
    float m_tiempoResultado = 0.0f;
    std::string m_mensajeResultado;
    
    // Generador aleatorio
    std::random_device m_rd;
    std::mt19937 m_gen;
    
    void inicializarUI();
    void inicializarColores();
    void generarNuevoPedido();
    void mezclarColor(ColorBebida color);
    void verificarPedido();
    void resetearMezcla();
    
    sf::Color obtenerColorSFML(ColorBebida tipo);
    sf::Color mezclarColores(sf::Color a, sf::Color b);
    bool coloresIguales(const sf::Color& a, const sf::Color& b);
};

#endif