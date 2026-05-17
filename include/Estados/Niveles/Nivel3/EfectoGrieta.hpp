#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>

// Clase que gestiona el efecto visual de una grieta en la pared
// Crea un agujero negro irregular con lineas de fractura y escombros
class EfectoGrieta {
private:
    // Lineas que simulan las fracturas en la pared
    std::vector<sf::VertexArray> lineasGrieta;
    
    // Pequenos fragmentos de escombro alrededor del agujero
    std::vector<sf::CircleShape> escombros;
    
    // Rectangulo negro base que representa el agujero
    sf::RectangleShape agujeroOscuro;
    
    // Posicion y tamano del area de la grieta
    sf::Vector2f posicion;
    sf::Vector2f tamano;
    
    // Control de animacion
    bool activo;
    float progresoAnimacion;  // De 0.0 (no visible) a 1.0 (completamente visible)
    
    // Borde irregular del agujero para que no sea un rectangulo perfecto
    sf::VertexArray bordeAgujero;
    
public:
    EfectoGrieta();
    
    // Inicializa la grieta en una posicion de la pared con un tamano especifico
    void iniciar(const sf::Vector2f& posicionPared, const sf::Vector2f& tamanoPared);
    
    // Actualiza la animacion de aparicion de la grieta
    void actualizar(float dt);
    
    // Dibuja la grieta en la ventana
    void dibujar(sf::RenderWindow& ventana);
    
    bool estaActivo() const { return activo; }
    void establecerProgresoAnimacion(float progreso) { progresoAnimacion = progreso; }
};