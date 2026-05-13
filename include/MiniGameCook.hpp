#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include "Inventory.hpp"

// Estructura para ingrediente
struct Ingrediente {
    std::string nombre;
    std::string categoria;
    sf::Color color;
    std::string rutaImagen;
    
    Ingrediente(const std::string& n, const std::string& cat, const sf::Color& col, const std::string& ruta = "")
        : nombre(n), categoria(cat), color(col), rutaImagen(ruta) {}
};

// Estructura para receta/plato
struct Plato {
    std::string nombre;
    std::vector<std::string> ingredientesNecesarios;
    std::string descripcion;
    sf::Color color;
    
    Plato(const std::string& n, const std::vector<std::string>& ingredientes, const std::string& desc, const sf::Color& col)
        : nombre(n), ingredientesNecesarios(ingredientes), descripcion(desc), color(col) {}
};

class MiniGameCook {
private:
    enum class EstadoMinijuego {
        ESPERANDO,
        MOSTRANDO_ESTANTE,
        COCINANDO,
        ENTREGANDO
    };
    
    EstadoMinijuego m_estadoActual;
    
    struct Estante {
        std::string nombre;
        std::vector<Ingrediente> ingredientes;
        sf::FloatRect areaInteraccion;
        bool visible;
    };
    
    std::vector<Estante> m_estantes;
    Estante* m_estanteActual;
    int m_indiceSeleccionado;
    
    sf::FloatRect m_areaCocina;
    sf::FloatRect m_areaEntrega;
    sf::FloatRect m_areaMenu;
    
    Plato m_platoRequerido;
    std::vector<Plato> m_recetasDisponibles;
    bool m_platoEntregado;
    bool m_misionCompletada;
    
    bool m_mostrarUIEstante;
    
    std::unique_ptr<sf::Font> m_font;
    std::unique_ptr<sf::Text> m_tituloEstante;
    std::vector<std::unique_ptr<sf::Text>> m_textosIngredientes;
    std::unique_ptr<sf::Text> m_textoInstrucciones;
    
    Inventory* m_inventory;
    std::map<std::string, sf::Texture> m_texturasIngredientes;
    std::function<void(const std::string&, float, sf::Color)> m_mensajeCallback;

    void inicializarIngredientes();
    void inicializarRecetas();
    void cargarTexturas();
    bool tieneIngredientes(const Plato& plato);
    bool consumirIngredientes(const Plato& plato);
    void agregarPlatoAlInventario(const Plato& plato);
    
public:
    MiniGameCook(Inventory* inventory);
    ~MiniGameCook() = default;
    void setMensajeCallback(std::function<void(const std::string&, float, sf::Color)> callback) 
    {
        m_mensajeCallback = callback;
    }
    void actualizarAreaEstante(const std::string& nombre, const sf::FloatRect& area);
    void update(float dt, const sf::Vector2f& playerPos);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::Vector2f& playerPos);
    void draw(sf::RenderWindow& window);
    
    void setAreaCocina(const sf::FloatRect& area) { m_areaCocina = area; }
    void setAreaEntrega(const sf::FloatRect& area) { m_areaEntrega = area; }
    void setAreaMenu(const sf::FloatRect& area) { m_areaMenu = area; }
    void agregarEstante(const std::string& nombre, const sf::FloatRect& area);
    void agregarIngredienteAEstante(const std::string& nombreEstante, const Ingrediente& ingrediente);
    
    bool isMisionCompletada() const { return m_misionCompletada; }
    void reiniciarMision();
    
    bool estaCercaCocina(const sf::Vector2f& playerPos) const;
    bool estaCercaEntrega(const sf::Vector2f& playerPos) const;
    bool estaCercaMenu(const sf::Vector2f& playerPos) const;
    std::string getEstanteCerca(const sf::Vector2f& playerPos) const;
    
    std::string getPlatoRequeridoNombre() const { return m_platoRequerido.nombre; }
    std::string getPlatoRequeridoDescripcion() const { return m_platoRequerido.descripcion; }
    std::vector<std::string> getIngredientesRequeridos() const { 
        return m_platoRequerido.ingredientesNecesarios; 
    }
};

class CocinaMinigameState {
private:
    MiniGameCook m_miniGame;
    bool m_activo;
    std::unique_ptr<sf::Font> m_font;
    std::unique_ptr<sf::Text> m_textoFeedback;
    bool m_fontLoaded;
    
public:
    CocinaMinigameState(Inventory* inventory);
    void update(float dt, const sf::Vector2f& playerPos);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, const sf::Vector2f& playerPos);
    void draw(sf::RenderWindow& window);
    
    void setAreas(const sf::FloatRect& cocina, const sf::FloatRect& entrega, const sf::FloatRect& menu);
    void agregarEstante(const std::string& nombre, const sf::FloatRect& area);
    
    bool isMisionCompletada() const { return m_miniGame.isMisionCompletada(); }
    void activar() { m_activo = true; }
    void desactivar() { m_activo = false; }
    bool isActivo() const { return m_activo; }
    MiniGameCook* getMiniGame() { return &m_miniGame; }
};