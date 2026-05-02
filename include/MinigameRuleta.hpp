#ifndef MINIGAMERULETA_HPP
#define MINIGAMERULETA_HPP

#include <SFML/Graphics.hpp>
#include <random>
#include <memory>
#include <vector>

class MinigameRuleta {
public:
    MinigameRuleta();
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    
    void activate();
    void deactivate();
    bool isActive() const { return m_isActive; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    // Devuelve true si ganó, y cuánto
    struct ResultadoApuesta {
        bool gano = false;
        int ganancia = 0;
        std::string mensaje;
    };
    
    // Para acceder al dinero del jugador
    void setDineroJugador(int* dinero) { m_dineroJugador = dinero; }
    int* getDineroJugador() { return m_dineroJugador; }
    
private:
    bool m_isActive = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // Fondo del minijuego
    sf::RectangleShape m_background;
    
    // Textura de la ruleta
    sf::Texture m_ruletaTexture;
    std::unique_ptr<sf::Sprite> m_ruletaSprite;
    
    // Estados de apuesta
    enum class TipoApuesta {
        NINGUNA,
        NUMERO_EXACTO,      // 35:1
        CUARTO,             // 3:1  (1-12, 13-24, 25-36)
        COLOR               // 2:1  (rojo/negro/verde)
    };
    
    TipoApuesta m_tipoApuesta = TipoApuesta::NINGUNA;
    
    // Para número exacto
    int m_numeroElegido = 0;
    
    // Para cuarto
    int m_cuartoElegido = 0; // 0, 1, 2
    
    // Para color
    enum class ColorElegido { NINGUNO, ROJO, NEGRO, VERDE };
    ColorElegido m_colorElegido = ColorElegido::NINGUNO;
    
    // Cantidad apostada
    int m_apuesta = 5;
    int m_apuestaMinima = 5;
    int m_apuestaMaxima = 100;
    
    // Dinero del jugador (puntero al dinero del nivel)
    int* m_dineroJugador = nullptr;
    
    // Resultado
    int m_numeroGanador = -1;
    bool m_mostrandoResultado = false;
    float m_tiempoResultado = 0.0f;
    std::string m_mensajeResultado;
    
    // Fuente
    sf::Font m_font;
    bool m_fontLoaded = false;
    
    // Textos
    std::unique_ptr<sf::Text> m_tituloText;
    std::unique_ptr<sf::Text> m_instruccionesText;
    std::unique_ptr<sf::Text> m_apuestaText;
    std::unique_ptr<sf::Text> m_resultadoText;
    std::unique_ptr<sf::Text> m_dineroText;
    
    // Botones
    sf::RectangleShape m_btnNumeroExacto;
    sf::RectangleShape m_btnCuarto;
    sf::RectangleShape m_btnColor;
    sf::RectangleShape m_btnGirar;
    sf::RectangleShape m_btnAumentar;
    sf::RectangleShape m_btnDisminuir;
    
    std::unique_ptr<sf::Text> m_textoBtnExacto;
    std::unique_ptr<sf::Text> m_textoBtnCuarto;
    std::unique_ptr<sf::Text> m_textoBtnColor;
    std::unique_ptr<sf::Text> m_textoBtnGirar;
    std::unique_ptr<sf::Text> m_textoBtnAumentar;
    std::unique_ptr<sf::Text> m_textoBtnDisminuir;
    
    // Selector de número (para apuesta exacta)
    std::vector<sf::RectangleShape> m_botonesNumeros;
    std::vector<std::unique_ptr<sf::Text>> m_textosNumeros;
    std::vector<bool> m_hoverNumeros;
    bool m_mostrandoTableroNumeros = false;
    
    // Selector de cuarto
    sf::RectangleShape m_btnCuarto1;
    sf::RectangleShape m_btnCuarto2;
    sf::RectangleShape m_btnCuarto3;
    std::unique_ptr<sf::Text> m_textoCuarto1;
    std::unique_ptr<sf::Text> m_textoCuarto2;
    std::unique_ptr<sf::Text> m_textoCuarto3;
    bool m_mostrandoSelectorCuarto = false;
    
    // Selector de color
    sf::RectangleShape m_btnRojo;
    sf::RectangleShape m_btnNegro;
    sf::RectangleShape m_btnVerde;
    std::unique_ptr<sf::Text> m_textoRojo;
    std::unique_ptr<sf::Text> m_textoNegro;
    std::unique_ptr<sf::Text> m_textoVerde;
    bool m_mostrandoSelectorColor = false;
    
    // Hover
    bool m_hoverExacto = false;
    bool m_hoverCuarto = false;
    bool m_hoverColor = false;
    bool m_hoverGirar = false;
    bool m_hoverAumentar = false;
    bool m_hoverDisminuir = false;
    bool m_hoverCuarto1 = false;
    bool m_hoverCuarto2 = false;
    bool m_hoverCuarto3 = false;
    bool m_hoverRojo = false;
    bool m_hoverNegro = false;
    bool m_hoverVerde = false;
    
    // Generador aleatorio
    std::random_device m_rd;
    std::mt19937 m_gen;
    
    void inicializarUI();
    void girarRuleta();
    int calcularGanancia();
    bool esRojo(int numero);
    bool esNegro(int numero);
    int obtenerCuarto(int numero);
};

#endif