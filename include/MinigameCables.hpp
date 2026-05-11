#ifndef MINIGAMECABLES_HPP
#define MINIGAMECABLES_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <random>

class MinigameCables {
public:
    MinigameCables();
    
    void setSize(const sf::Vector2f& size);
    void setPosition(const sf::Vector2f& position);
    void activate();
    void deactivate();
    bool isActive() const { return m_active; }
    
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }
    void setOnFail(std::function<void()> callback) { m_onFail = callback; }
    
    bool isCompleted() const { return m_completed; }
    void setOrdenCorrecto(const std::vector<int>& orden) { m_ordenPredefinido = orden; }
    void setPista1(const std::string& pista) { m_pista1 = pista; }
    void setPista2(const std::string& pista) { m_pista2 = pista; }

private:
    struct Cable {
        std::unique_ptr<sf::RectangleShape> formaCable;
        std::unique_ptr<sf::RectangleShape> puntaIzquierda;
        std::unique_ptr<sf::RectangleShape> puntaDerecha;
        sf::Color color;
        std::string nombreColor;
        bool cortado = false;
        bool animandoCorte = false;
        float tiempoAnimacion = 0.0f;
        int numero;
        std::unique_ptr<sf::Text> textoNumero;
        std::vector<sf::CircleShape> chispas;
        float tiempoChispas = 0.0f;
    };
    
    bool m_active = false;
    bool m_completed = false;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    std::vector<Cable> m_cables;
    std::vector<int> m_ordenCorrecto;
    std::vector<int> m_ordenPredefinido;
    int m_cablesCortados = 0;
    int m_maxErrores = 3;
    int m_errores = 0;
    
    sf::Font m_font;
    bool m_fontLoaded = false;
    std::unique_ptr<sf::Text> m_textoInstrucciones;
    std::unique_ptr<sf::Text> m_textoEstado;
    std::unique_ptr<sf::Text> m_textoErrores;
    std::unique_ptr<sf::Text> m_textoPista1;
    std::unique_ptr<sf::Text> m_textoPista2;
    
    sf::RectangleShape m_background;
    sf::RectangleShape m_panel;
    std::vector<sf::Vertex> m_rayosElectricos;
    float m_tiempoRayos = 0.0f;
    float m_tiempoParpadeo = 0.0f;
    bool m_parpadeoVisible = true;
    
    bool m_mostrarTijeras = false;
    float m_tiempoTijeras = 0.0f;
    sf::Vector2f m_posicionTijeras;
    sf::RectangleShape m_tijeraHoja1;
    sf::RectangleShape m_tijeraHoja2;
    sf::CircleShape m_tijeraPivote;
    
    bool m_gameOver = false;
    float m_tiempoGameOver = 0.0f;
    sf::RectangleShape m_overlayGameOver;
    
    std::function<void()> m_onComplete;
    std::function<void()> m_onFail;
    
    std::string m_pista1 = "";
    std::string m_pista2 = "";
    
    std::mt19937 m_gen;
    std::uniform_real_distribution<float> m_dist;
    
    void inicializarCables();
    void cortarCable(int indice);
    void verificarVictoria();
    void crearCable(sf::Color color, const std::string& nombre, int numero, float x, float y);
    void actualizarAnimacionCorte(Cable& cable, float dt);
    void crearChispas(Cable& cable);
    void dibujarTijeras(sf::RenderWindow& window);
    void actualizarRayosElectricos(float dt);
    void actualizarParpadeo(float dt);
    
    
sf::Texture m_texturaFondo;
std::unique_ptr<sf::Sprite> m_spriteFondo;
bool m_fondoCargado = false;
};

#endif