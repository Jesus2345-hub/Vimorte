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
    
    struct ResultadoApuesta {
        bool gano = false;
        int ganancia = 0;
        std::string mensaje;
    };
    
    void setDineroJugador(int* dinero) { m_dineroJugador = dinero; }
    int* getDineroJugador() { return m_dineroJugador; }
    
private:
    // Animación de la ruleta
    bool m_girando = false;
    float m_anguloActual = 0.f;
    float m_velocidadGiro = 0.f;
    float m_anguloObjetivo = 0.f;
    sf::CircleShape m_circuloRuleta;
    std::vector<sf::Text> m_numerosRuleta;
    sf::RectangleShape m_flecha;
    std::vector<int> m_ordenRuleta;
    
    void inicializarRuletaAnimada();
    
    // Animación de la pelotita
    bool m_pelotitaGirando = false;
    float m_anguloPelotita = 0.f;
    float m_velocidadPelotita = 0.f;
    float m_anguloObjetivoPelotita = 0.f;
    sf::CircleShape m_pelotita;
    
    bool m_isActive = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    sf::RectangleShape m_background;
    sf::Texture m_ruletaTexture;
    std::unique_ptr<sf::Sprite> m_ruletaSprite;
    
    enum class TipoApuesta {
        NINGUNA,
        NUMERO_EXACTO,      // 35:1
        CUARTO,             // 3:1 (1-12, 13-24, 25-36) - DOCENAS
        COLUMNA,            // 2:1
        COLOR,              // 2:1 (rojo/negro) o 36:1 (verde)
        PAR_IMPAR,          // 1:1
        MANQUE_PASSE        // 1:1 (1-18 / 19-36)
    };
    
    TipoApuesta m_tipoApuesta = TipoApuesta::NINGUNA;
    
    int m_numeroElegido = -1;
    int m_cuartoElegido = -1; // 0, 1, 2 (docenas)
    
    enum class ColorElegido { NINGUNO, ROJO, NEGRO, VERDE };
    ColorElegido m_colorElegido = ColorElegido::NINGUNO;
    
    // Par/Impar
    int m_parImparElegido = -1; // 0 = Par, 1 = Impar
    
    // Columna y Mitad
    int m_columnaElegida = -1;   // 0, 1, 2 para las 3 columnas
    int m_mitadElegida = -1;     // 0 = 1-18, 1 = 19-36
    
    int m_apuesta = 5;
    int m_apuestaMinima = 5;
    int m_apuestaMaxima = 100;
    
    int* m_dineroJugador = nullptr;
    
    int m_numeroGanador = -1;
    bool m_mostrandoResultado = false;
    float m_tiempoResultado = 0.0f;
    std::string m_mensajeResultado;
    
    sf::Font m_font;
    bool m_fontLoaded = false;
    
    std::unique_ptr<sf::Text> m_tituloText;
    std::unique_ptr<sf::Text> m_instruccionesText;
    std::unique_ptr<sf::Text> m_apuestaText;
    std::unique_ptr<sf::Text> m_resultadoText;
    std::unique_ptr<sf::Text> m_dineroText;
    
    // Botones de tipo de apuesta
    sf::RectangleShape m_btnNumeroExacto;
    sf::RectangleShape m_btnCuarto;
    sf::RectangleShape m_btnColor;
    sf::RectangleShape m_btnParImpar;
    sf::RectangleShape m_btnGirar;
    sf::RectangleShape m_btnAumentar;
    sf::RectangleShape m_btnDisminuir;
    
    std::unique_ptr<sf::Text> m_textoBtnExacto;
    std::unique_ptr<sf::Text> m_textoBtnCuarto;
    std::unique_ptr<sf::Text> m_textoBtnColor;
    std::unique_ptr<sf::Text> m_textoBtnParImpar;
    std::unique_ptr<sf::Text> m_textoBtnGirar;
    std::unique_ptr<sf::Text> m_textoBtnAumentar;
    std::unique_ptr<sf::Text> m_textoBtnDisminuir;
    
    // Selector de número
    std::vector<sf::RectangleShape> m_botonesNumeros;
    std::vector<std::unique_ptr<sf::Text>> m_textosNumeros;
    std::vector<bool> m_hoverNumeros;
    bool m_mostrandoTableroNumeros = false;
    
    // Selector de cuarto/docenas
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
    
    // Selector de Par/Impar
    sf::RectangleShape m_btnPar;
    sf::RectangleShape m_btnImpar;
    std::unique_ptr<sf::Text> m_textoPar;
    std::unique_ptr<sf::Text> m_textoImpar;
    bool m_mostrandoSelectorParImpar = false;
    
    // ===== TAPETE DE RULETA INTERACTIVO =====
    std::vector<sf::RectangleShape> m_celdasTapete;      // Celdas del tapete (0-36)
    std::vector<std::unique_ptr<sf::Text>> m_textosTapete;
    std::vector<bool> m_hoverCeldas;
    
    // Botones para DOCENAS (1st 12, 2nd 12, 3rd 12)
    sf::RectangleShape m_btnDocena1;
    sf::RectangleShape m_btnDocena2;
    sf::RectangleShape m_btnDocena3;
    std::unique_ptr<sf::Text> m_textoDocena1;
    std::unique_ptr<sf::Text> m_textoDocena2;
    std::unique_ptr<sf::Text> m_textoDocena3;
    
    // Botones para COLUMNAS (2 to 1)
    sf::RectangleShape m_btnColumna1;
    sf::RectangleShape m_btnColumna2;
    sf::RectangleShape m_btnColumna3;
    std::unique_ptr<sf::Text> m_textoColumna1;
    std::unique_ptr<sf::Text> m_textoColumna2;
    std::unique_ptr<sf::Text> m_textoColumna3;
    
    // Botones para MITADES (1-18 / 19-36)
    sf::RectangleShape m_btnMitadBaja;   // 1 to 18
    sf::RectangleShape m_btnMitadAlta;   // 19 to 36
    std::unique_ptr<sf::Text> m_textoMitadBaja;
    std::unique_ptr<sf::Text> m_textoMitadAlta;
    
    // Variables para el estado del tapete
    bool m_mostrandoTapete = true;  // Siempre visible
    float m_tapeteX = 0.f;
    float m_tapeteY = 0.f;
    float m_celdaWidth = 0.f;
    float m_celdaHeight = 0.f;
    
    // Hover
    bool m_hoverExacto = false;
    bool m_hoverCuarto = false;
    bool m_hoverColor = false;
    bool m_hoverParImpar = false;
    bool m_hoverGirar = false;
    bool m_hoverAumentar = false;
    bool m_hoverDisminuir = false;
    bool m_hoverCuarto1 = false;
    bool m_hoverCuarto2 = false;
    bool m_hoverCuarto3 = false;
    bool m_hoverRojo = false;
    bool m_hoverNegro = false;
    bool m_hoverVerde = false;
    bool m_hoverPar = false;
    bool m_hoverImpar = false;
    bool m_hoverColumna1 = false;
    bool m_hoverColumna2 = false;
    bool m_hoverColumna3 = false;
    bool m_hoverMitadBaja = false;
    bool m_hoverMitadAlta = false;
    bool m_hoverDocena1 = false;
    bool m_hoverDocena2 = false;
    bool m_hoverDocena3 = false;
    
    std::random_device m_rd;
    std::mt19937 m_gen;
    
    void inicializarUI();
    void inicializarTapete();
    void girarRuleta();
    int calcularGanancia();
    bool esRojo(int numero);
    bool esNegro(int numero);
    int obtenerCuarto(int numero);
    bool esPar(int numero);
    int obtenerColumna(int numero);
};

#endif