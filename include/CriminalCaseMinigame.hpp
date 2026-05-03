#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>

// Estructura para cada objeto a encontrar
struct ObjetoBuscar {
    std::string nombre;
    sf::FloatRect area;
    bool encontrado;
    std::string descripcion;
    std::string imagenPath;
    
    // Textura y sprite del objeto (unique_ptr)
    std::unique_ptr<sf::Texture> textura;
    std::unique_ptr<sf::Sprite> sprite;
    
    ObjetoBuscar(const std::string& n, const sf::FloatRect& a, const std::string& img, const std::string& d)
        : nombre(n), area(a), encontrado(false), descripcion(d), imagenPath(img)
        , textura(nullptr), sprite(nullptr) {}
    
    // Constructor de movimiento
    ObjetoBuscar(ObjetoBuscar&& other) noexcept = default;
    ObjetoBuscar& operator=(ObjetoBuscar&& other) noexcept = default;
    
    // Cargar textura y sprite
    bool cargarTextura() {
        textura = std::make_unique<sf::Texture>();
        if (!textura->loadFromFile(imagenPath)) {
            return false;
        }
        sprite = std::make_unique<sf::Sprite>(*textura);
        sprite->setPosition(area.position);
        
        // Escalar al tamaño del área
        sf::Vector2u texSize = textura->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            sprite->setScale(sf::Vector2f(
                area.size.x / texSize.x,
                area.size.y / texSize.y
            ));
        }
        return true;
    }
};

// Estructura para cada sospechoso
struct Sospechoso {
    std::string nombre;
    sf::FloatRect area;
    bool acusado;
    std::string descripcion;
    bool esElCulpable;
    
    Sospechoso(const std::string& n, const sf::FloatRect& a, const std::string& d, bool culpable)
        : nombre(n), area(a), acusado(false), descripcion(d), esElCulpable(culpable) {}
};

class CriminalCaseMinigame {
public:
    CriminalCaseMinigame();
    ~CriminalCaseMinigame() = default;
    
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void setDineroJugador(int* dinero) { m_dineroJugador = dinero; }
    
    void init(const std::string& fondoPath,
              std::vector<ObjetoBuscar> objetos,
              std::vector<Sospechoso> sospechosos);
    
    void activate();
    void deactivate();
    bool isActive() const { return m_active; }
    bool isComplete() const { return m_completed; }
    
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    
    void setOnCompleteCallback(std::function<void(bool)> callback) { 
        m_onCompleteCallback = callback; 
    }
    
    void setDebugMode(bool debug) { m_debugMode = debug; }
    
private:
    void mostrarMensaje(const std::string& msg, float duracion);
    void verificarCompletado();
    int contarObjetosEncontrados() const;
    void updateListaTexto();
    void cargarFuente();
    void centrarTexto(sf::Text& text, float x, float y);
    
    bool m_active;
    bool m_todasEvidencias;
    bool m_culpableEncontrado;
    bool m_completed;
    bool m_debugMode;
    bool m_fontLoaded;
    
    int* m_dineroJugador;
    
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    
    // Fondo del minijuego (inicializados como nullptr)
    std::unique_ptr<sf::Texture> m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    
    // Lista de objetos a buscar
    std::vector<ObjetoBuscar> m_objetos;
    std::vector<Sospechoso> m_sospechosos;
    
    // Textos UI (inicializados como nullptr)
    std::unique_ptr<sf::Font> m_font;
    std::unique_ptr<sf::Text> m_mensajeText;
    std::unique_ptr<sf::Text> m_listaText;
    std::unique_ptr<sf::Text> m_instruccionText;
    
    struct MensajeTemp {
        std::string texto;
        float tiempoRestante;
        MensajeTemp() : tiempoRestante(0.0f) {}
    };
    MensajeTemp m_mensajeTemp;
    sf::Clock m_mensajeClock;
    
    std::function<void(bool)> m_onCompleteCallback;
};