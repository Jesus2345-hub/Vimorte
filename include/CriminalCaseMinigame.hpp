#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <random>
#include <iostream>
#include "Inventory.hpp" 

// OBJETOS
struct ObjetoBuscar {
    std::string nombre;
    sf::FloatRect area;
    bool encontrado;
    std::string rutaImagen;
    std::string descripcion;
    
    ObjetoBuscar() : encontrado(false) {}
    ObjetoBuscar(const std::string& n, const sf::FloatRect& a, const std::string& d, const std::string& img)
        : nombre(n), area(a), encontrado(false), descripcion(d), rutaImagen(img) {}
};

struct Sospechoso {
    std::string nombre;
    sf::FloatRect area;
    bool acusado;
    std::string descripcion;
    bool esElCulpable;
    
    Sospechoso() : acusado(false), esElCulpable(false) {}
    Sospechoso(const std::string& n, const sf::FloatRect& a, const std::string& d, bool culpable)
        : nombre(n), area(a), acusado(false), descripcion(d), esElCulpable(culpable) {}
};

// ESTADOS DEL MINIJUEGO
enum class CriminalGameState {
    BUSCANDO_EVIDENCIAS,  
    NARRATIVA,             
    ELECCION_FINAL         
};

// DIALOGO NARRATIVO 
struct DialogoNarrativo {
    std::string persona;
    std::string texto;
    std::string fondoPath;  
    std::shared_ptr<sf::Texture> fondoTexture;  
    bool fondoCargado;
    
    DialogoNarrativo() : fondoCargado(false) {}
    DialogoNarrativo(const std::string& p, const std::string& t) 
        : persona(p), texto(t), fondoPath(""), fondoCargado(false) {}
    DialogoNarrativo(const std::string& p, const std::string& t, const std::string& path)
        : persona(p), texto(t), fondoPath(path), fondoCargado(false) {}
};

//  CLASE PRINCIPAL 
class CriminalCaseMinigame {
public:
   
    CriminalCaseMinigame();
    ~CriminalCaseMinigame() = default;

    //  GETTERS Y SETTERS 
    int getSetActual() const { return m_setActualObjetos; }
    int getSetActualObjetos() const { return m_setActualObjetos; }
    bool isActive() const { return m_active; }
    bool isComplete() const { return m_completed; }
    void setInventory(Inventory* inventory) { m_inventory = inventory; }
    void setDebugMode(bool debug) { m_debugMode = debug; }
    void setOnCompleteCallback(std::function<void(bool)> callback) { m_onCompleteCallback = callback; }
    void setPenaltyCallback(std::function<void()> callback) { m_penaltyCallback = callback; }
    
    //  CONFIGURACIÓN Y POSICION 
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);
    void setBaseSize(const sf::Vector2f& baseSize);
    void cargarFondoOnly(const std::string& fondoPath);
    
    //  MANEJO DE CASOS
    void limpiarPools();
    void agregarSetObjetos(const std::vector<ObjetoBuscar>& objetos);
    void agregarSetSospechosos(const std::vector<Sospechoso>& sospechosos);
    void agregarSetDialogos(const std::vector<DialogoNarrativo>& dialogos);
    void generarNuevoCaso();
    void setDialogos(const std::vector<DialogoNarrativo>& dialogos);
    
    //  INICIALIZACIÓN Y RESET 
    void init(const std::string& fondoPath, std::vector<ObjetoBuscar> objetos, std::vector<Sospechoso> sospechosos);
    void reinit(const std::string& fondoPath, const std::vector<ObjetoBuscar>& objetos, const std::vector<Sospechoso>& sospechosos);
    void resetCompletamente();
    void resetGame();
    void escalarAreas();
    
    //  CONTROL DEL MINIJUEGO 
    void activate();
    void deactivate();
    void limpiarInventario();
    
    //  CICLO DE VIDEOJUEGO 
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    
    void actualizarFondo();
    void mostrarMensaje(const std::string& msg, float duracion);
    void mostrarMensajeConFondo(const std::string& msg, float duracion, sf::Color color);
    void verificarCompletado();
    int contarObjetosEncontrados() const;
    void updateListaTexto();
    void cargarFuente();
    void cargarFondoDialogo(DialogoNarrativo& dialogo);
    void centrarTexto(sf::Text& text, float x, float y);
    void recalcularAreasBotones();
    void reiniciarCasoActual();  
    void limpiarInventarioCaso();
    void generarNuevoCasoCompleto();
    void iniciarFaseNarrativa();
    void avanzarDialogo();
    void retrocederDialogo();
    void dibujarPantallaNarrativa(sf::RenderWindow& window);
    void dibujarPantallaEleccion(sf::RenderWindow& window);
    void procesarAcusacion(int sospechosoIndex);
    void penalizarJugador();
    void reiniciarCasoCompleto();

    //  VARIABLES DE ESTADO DEL JUEGO 
    bool m_mensajeErrorActivo;
    bool m_active;                  
    bool m_todasEvidencias;         
    bool m_culpableEncontrado;      
    bool m_completed;               
    bool m_debugMode;               
    bool m_fontLoaded;              
    bool m_waitingForNarrative;     
    
    CriminalGameState m_gameState; 
    Inventory* m_inventory;        
    
    //  POSICIÓN Y TAMAÑO 
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Vector2f m_tamanioBase;   
    
    //  FONDO 
    std::string m_fondoPath;
    std::unique_ptr<sf::Texture> m_backgroundTexture;
    std::unique_ptr<sf::Sprite> m_background;
    
    //  DATOS DEL CASO ACTUAL 
    std::vector<ObjetoBuscar> m_objetos;           // Objetos actuales
    std::vector<Sospechoso> m_sospechosos;         // Sospechosos actuales
    std::vector<ObjetoBuscar> m_objetosOriginales; // Copia original
    std::vector<Sospechoso> m_sospechososOriginales;// Copia original
    
    //  POOLS DE DATOS  
    std::vector<std::vector<ObjetoBuscar>> m_poolObjetos;
    std::vector<std::vector<Sospechoso>> m_poolSospechosos;
    std::vector<std::vector<DialogoNarrativo>> m_poolDialogos;
    
    //  INDICES DEL CASO ACTUAL 
    int m_setActualObjetos;
    int m_setActualSospechosos;
    int m_setActualDialogos;
    
    //  DIALOGOS NARRATIVOS 
    std::vector<DialogoNarrativo> m_dialogosActuales;
    int m_dialogoActualIndex;
    sf::RectangleShape m_fondoNarrativo;
    sf::RectangleShape m_cuadroDialogo;
    
    //  UI TEXTOS 
    std::unique_ptr<sf::Font> m_font;
    std::unique_ptr<sf::Text> m_mensajeText;
    std::unique_ptr<sf::Text> m_listaText;          
    std::unique_ptr<sf::Text> m_instruccionText;   
    std::unique_ptr<sf::Text> m_dialogoText;        
    std::unique_ptr<sf::Text> m_personaText;        
    std::unique_ptr<sf::Text> m_instruccionesNarrativas;
    std::unique_ptr<sf::Text> m_tituloEleccion;     
    std::unique_ptr<sf::Text> m_mensajeAdvertencia; 
    
    //  BOTONES DE ELECCIÓN 
    std::vector<sf::FloatRect> m_areasBotones;
    
    //  MENSAJES TEMPORALES 
    struct MensajeTemp {
        std::string texto;
        float tiempoRestante;
        sf::Color color;
        MensajeTemp() : tiempoRestante(0.0f), color(sf::Color::Yellow) {}
    };
    MensajeTemp m_mensajeTemp;
    sf::Clock m_mensajeClock;
    
    //  CALLBACKS 
    std::function<void(bool)> m_onCompleteCallback;  
    std::function<void()> m_penaltyCallback;         
    
    //  GENERAR NUMEROS ALEATORIOS 
    std::mt19937 m_rng;  
};