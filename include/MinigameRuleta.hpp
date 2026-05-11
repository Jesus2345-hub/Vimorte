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
        CUARTO,             // 3:1  (1-12, 13-24, 25-36)
        COLOR,              // 2:1  (rojo/negro/verde)
        PAR_IMPAR           // 1:1  (par/impar)
    };
    
    TipoApuesta m_tipoApuesta = TipoApuesta::NINGUNA;
    
    int m_numeroElegido = 0;
    int m_cuartoElegido = 0; // 0, 1, 2
    
    enum class ColorElegido { NINGUNO, ROJO, NEGRO, VERDE };
    ColorElegido m_colorElegido = ColorElegido::NINGUNO;
    
    // Par/Impar
    int m_parImparElegido = -1; // 0 = Par, 1 = Impar
    
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
    sf::RectangleShape m_btnParImpar;            // NUEVO
    sf::RectangleShape m_btnGirar;
    sf::RectangleShape m_btnAumentar;
    sf::RectangleShape m_btnDisminuir;
    
    std::unique_ptr<sf::Text> m_textoBtnExacto;
    std::unique_ptr<sf::Text> m_textoBtnCuarto;
    std::unique_ptr<sf::Text> m_textoBtnColor;
    std::unique_ptr<sf::Text> m_textoBtnParImpar; // NUEVO
    std::unique_ptr<sf::Text> m_textoBtnGirar;
    std::unique_ptr<sf::Text> m_textoBtnAumentar;
    std::unique_ptr<sf::Text> m_textoBtnDisminuir;
    
    // Selector de número
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
    
    // Selector de Par/Impar (NUEVO)
    sf::RectangleShape m_btnPar;
    sf::RectangleShape m_btnImpar;
    std::unique_ptr<sf::Text> m_textoPar;
    std::unique_ptr<sf::Text> m_textoImpar;
    bool m_mostrandoSelectorParImpar = false;
    
    // Hover
    bool m_hoverExacto = false;
    bool m_hoverCuarto = false;
    bool m_hoverColor = false;
    bool m_hoverParImpar = false;   // NUEVO
    bool m_hoverGirar = false;
    bool m_hoverAumentar = false;
    bool m_hoverDisminuir = false;
    bool m_hoverCuarto1 = false;
    bool m_hoverCuarto2 = false;
    bool m_hoverCuarto3 = false;
    bool m_hoverRojo = false;
    bool m_hoverNegro = false;
    bool m_hoverVerde = false;
    bool m_hoverPar = false;        // NUEVO
    bool m_hoverImpar = false;      // NUEVO
    
    std::random_device m_rd;
    std::mt19937 m_gen;
    
    void inicializarUI();
    void girarRuleta();
    int calcularGanancia();
    bool esRojo(int numero);
    bool esNegro(int numero);
    int obtenerCuarto(int numero);
    bool esPar(int numero);          // Para la apuesta par/impar
};

#endif